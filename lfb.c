#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include <stdint.h>
#include <fcntl.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>

#include "lua.h"
#include "lauxlib.h"



#define VERSION "1.0"

#define FRAMEBUFFER_REGID "1992d3e1-6a72-4a4f-aaae-5159fc3a7728"

#define LUA_T_PUSH_S_N(S, N) lua_pushstring(L, S); lua_pushnumber(L, N); lua_settable(L, -3);
#define LUA_T_PUSH_S_S(S, S2) lua_pushstring(L, S); lua_pushstring(L, S2); lua_settable(L, -3);
#define LUA_T_PUSH_S_CF(S, CF) lua_pushstring(L, S); lua_pushcfunction(L, CF); lua_settable(L, -3);



typedef struct {
    int fd;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    char *fbdev;
    uint8_t *data;
} framebuffer_t;

typedef struct {
    uint8_t r, g, b, a;
} pixel_t;

typedef struct {
    int w, h;
    pixel_t *data;
} drawbuffer_t;



static inline uint32_t getcolor(framebuffer_t *lfb, uint8_t r, uint8_t g, uint8_t b) {
    switch (lfb->vinfo.bits_per_pixel) {
        case 16:
            return ((r >> (8 - lfb->vinfo.red.length)) << lfb->vinfo.red.offset) |
                ((g >> (8 - lfb->vinfo.green.length))  << lfb->vinfo.green.offset) |
                ((b >> (8 - lfb->vinfo.blue.length))   << lfb->vinfo.blue.offset);
        case 32:
            return (r << lfb->vinfo.red.offset) |
                (g << lfb->vinfo.green.offset) |
                (b << lfb->vinfo.blue.offset);
    }
    return -1;
}

static void getnumfield32(lua_State *L, const char *key, uint32_t *dest) {
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
        *dest = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
}



static int lfb_framebuffer_close(lua_State *L) {
    framebuffer_t *fb = (framebuffer_t *)lua_touserdata(L, 1);

    if (fb->fd >= 0) {
        close(fb->fd);
        fb->fd = -1;
        free(fb->fbdev);
    }

    return 0;
}

static int lfb_framebuffer_getvarinfo(lua_State *L) {
    framebuffer_t *fb = (framebuffer_t *)lua_touserdata(L, 1);

    ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vinfo);

    lua_newtable(L);

    LUA_T_PUSH_S_N("xres", fb->vinfo.xres);
    LUA_T_PUSH_S_N("yres", fb->vinfo.yres);
    LUA_T_PUSH_S_N("xres_virtual", fb->vinfo.xres_virtual);
    LUA_T_PUSH_S_N("yres_virtual", fb->vinfo.yres_virtual);
    LUA_T_PUSH_S_N("xoffset", fb->vinfo.xoffset);
    LUA_T_PUSH_S_N("yoffset", fb->vinfo.yoffset);
    LUA_T_PUSH_S_N("bits_per_pixel", fb->vinfo.bits_per_pixel);
    LUA_T_PUSH_S_N("grayscale", fb->vinfo.grayscale);
    LUA_T_PUSH_S_N("nonstd", fb->vinfo.nonstd);
    LUA_T_PUSH_S_N("activate", fb->vinfo.activate);
    LUA_T_PUSH_S_N("width", fb->vinfo.width);
    LUA_T_PUSH_S_N("height", fb->vinfo.height);

    LUA_T_PUSH_S_N("pixclock", fb->vinfo.pixclock);
    LUA_T_PUSH_S_N("left_margin", fb->vinfo.left_margin);
    LUA_T_PUSH_S_N("right_margin", fb->vinfo.right_margin);
    LUA_T_PUSH_S_N("upper_margin", fb->vinfo.upper_margin);
    LUA_T_PUSH_S_N("lower_margin", fb->vinfo.lower_margin);
    LUA_T_PUSH_S_N("hsync_len", fb->vinfo.hsync_len);
    LUA_T_PUSH_S_N("vsync_len", fb->vinfo.vsync_len);
    LUA_T_PUSH_S_N("sync", fb->vinfo.sync);
    LUA_T_PUSH_S_N("vmode", fb->vinfo.vmode);
    LUA_T_PUSH_S_N("rotate", fb->vinfo.rotate);
    LUA_T_PUSH_S_N("colorspace", fb->vinfo.colorspace);

    return 1;
}

static int lfb_framebuffer_getfixinfo(lua_State *L) {
    framebuffer_t *fb = (framebuffer_t *)lua_touserdata(L, 1);

    lua_newtable(L);

    LUA_T_PUSH_S_S("id", fb->finfo.id)
    LUA_T_PUSH_S_N("smem_start", fb->finfo.smem_start);
    LUA_T_PUSH_S_N("type", fb->finfo.type);
    LUA_T_PUSH_S_N("type_aux", fb->finfo.type_aux);
    LUA_T_PUSH_S_N("visual", fb->finfo.visual);
    LUA_T_PUSH_S_N("xpanstep", fb->finfo.xpanstep);
    LUA_T_PUSH_S_N("ypanstep", fb->finfo.ypanstep);
    LUA_T_PUSH_S_N("ywrapstep", fb->finfo.ywrapstep);
    LUA_T_PUSH_S_N("line_length", fb->finfo.line_length);
    LUA_T_PUSH_S_N("mmio_start", fb->finfo.mmio_start);
    LUA_T_PUSH_S_N("mmio_len", fb->finfo.mmio_len);
    LUA_T_PUSH_S_N("accel", fb->finfo.accel);
    LUA_T_PUSH_S_N("capabilities", fb->finfo.capabilities);

    return 1;
}

static int lfb_framebuffer_setvarinfo(lua_State *L) {
    framebuffer_t *fb = (framebuffer_t *)lua_touserdata(L, 1);

    ioctl(fb->fd, FBIOGET_VSCREENINFO, &fb->vinfo);

    getnumfield32(L, "xres", &fb->vinfo.xres);
    getnumfield32(L, "yres", &fb->vinfo.yres);
    getnumfield32(L, "xres_virtual", &fb->vinfo.xres_virtual);
    getnumfield32(L, "yres_virtual", &fb->vinfo.yres_virtual);
    getnumfield32(L, "xoffset", &fb->vinfo.xoffset);
    getnumfield32(L, "yoffset", &fb->vinfo.yoffset);
    getnumfield32(L, "bits_per_pixel", &fb->vinfo.bits_per_pixel);
    getnumfield32(L, "grayscale", &fb->vinfo.grayscale);
    getnumfield32(L, "nonstd", &fb->vinfo.nonstd);
    getnumfield32(L, "activate", &fb->vinfo.activate);
    getnumfield32(L, "width", &fb->vinfo.width);
    getnumfield32(L, "height", &fb->vinfo.height);

    getnumfield32(L, "pixclock", &fb->vinfo.pixclock);
    getnumfield32(L, "left_margin", &fb->vinfo.left_margin);
    getnumfield32(L, "right_margin", &fb->vinfo.right_margin);
    getnumfield32(L, "upper_margin", &fb->vinfo.upper_margin);
    getnumfield32(L, "lower_margin", &fb->vinfo.lower_margin);
    getnumfield32(L, "hsync_len", &fb->vinfo.hsync_len);
    getnumfield32(L, "vsync_len", &fb->vinfo.vsync_len);
    getnumfield32(L, "sync", &fb->vinfo.sync);
    getnumfield32(L, "vmode", &fb->vinfo.vmode);
    getnumfield32(L, "rotate", &fb->vinfo.rotate);
    getnumfield32(L, "colorspace", &fb->vinfo.colorspace);

    return 1;
}

static int lfb_framebuffer_tostring(lua_State *L) {
    framebuffer_t *fb = (framebuffer_t *)lua_touserdata(L, 1);

    if (fb->fbdev) {
        lua_pushfstring(L, "Framebuffer: %s (%s)", fb->fbdev, fb->finfo.id);
    } else {
        lua_pushfstring(L, "Closed framebuffer");
    }

    return 1;
}

static int lfb_framebuffer(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_newuserdata(L, sizeof(*lfb));

    lfb->fd = open(luaL_checkstring(L, 1), O_RDWR);
    if (lfb->fd < 0) {
        return luaL_error(L, "Couldn't open framebuffer: %s", strerror(errno));
    }

    ioctl(lfb->fd, FBIOGET_FSCREENINFO, &lfb->finfo);
    ioctl(lfb->fd, FBIOGET_VSCREENINFO, &lfb->vinfo);
    if (lfb->vinfo.bits_per_pixel != 16 && lfb->vinfo.bits_per_pixel != 32) {
        close(lfb->fd);
        lfb->fd = -1;
        return luaL_error(L, "Only 16 & 32 bpp are supported, not: %d", lfb->vinfo.bits_per_pixel);
    }
    lfb->data = mmap(0, (lfb->vinfo.yres_virtual * lfb->finfo.line_length), PROT_READ | PROT_WRITE, MAP_SHARED, lfb->fd, (off_t)0);
    lfb->fbdev = strdup(luaL_checkstring(L, 1));

    lua_createtable(L, 0, 9);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    
    LUA_T_PUSH_S_CF("close", lfb_framebuffer_close)
    LUA_T_PUSH_S_CF("get_fixinfo", lfb_framebuffer_getfixinfo)
    LUA_T_PUSH_S_CF("get_varinfo", lfb_framebuffer_getvarinfo)
    LUA_T_PUSH_S_CF("set_varinfo", lfb_framebuffer_setvarinfo)
    LUA_T_PUSH_S_CF("__gc", lfb_framebuffer_close)
    LUA_T_PUSH_S_CF("__tostring", lfb_framebuffer_tostring)
    lua_setmetatable(L, -2);

    return 1;
}


static int lfb_drawbuffer_tostring(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);

    lua_pushfstring(L, "Drawbuffer: %dx%d", db->w, db->h);

    return 1;
}

static int lfb_drawbuffer_close(lua_State *L) {
    // drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    
    // TODO

    return 0;
}

static int lfb_drawbuffer_clear(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    int r = lua_tointeger(L, 2);
    int g = lua_tointeger(L, 3);
    int b = lua_tointeger(L, 4);
    int a = lua_tointeger(L, 5);
    int y = 0;
    int x = 0;

    for (y = 0; y < db->h; y=y+1) {
        for (x = 0; x < db->w; x=x+1) {
            db->data[y*db->w+x] = (pixel_t) {.r=r, .g=g, .b=b, .a=a};
        }
    }

    return 0;
}

static int lfb_drawbuffer_pixel_function(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    pixel_t p;
    int x,y;
    
    if (lua_gettop(L) == 2 && lua_isfunction(L, 2)) {
        for (y = 0; y < db->h; y=y+1) {
            for (x = 0; x < db->w; x=x+1) {
                p = db->data[y*db->w+x];
                lua_pushnumber(L, x);
                lua_pushnumber(L, y);
                lua_pushnumber(L, p.r);
                lua_pushnumber(L, p.g);
                lua_pushnumber(L, p.b);
                lua_pushnumber(L, p.a);
                lua_pcall(L, 6, 4, 0);
                p.r = lua_tointeger(L, 1);
                p.g = lua_tointeger(L, 2);
                p.b = lua_tointeger(L, 3);
                p.a = lua_tointeger(L, 4);
                lua_pop(L, 4);
                db->data[y*db->w+x] = p;
            }
        }
    }
    return 0;

}

static int lfb_drawbuffer_draw_to_framebuffer(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    framebuffer_t *fb = (framebuffer_t *)lua_touserdata(L, 2);
    
    int x = lua_tointeger(L, 3);
    int y = lua_tointeger(L, 4);
    int cx;
    int cy;
    pixel_t p;
    uint32_t pixel;
    int location;

    for (cy=0; cy < db->h; cy=cy+1) {
        for (cx=0; cx < db->w; cx=cx+1) {
            p = db->data[cy*db->w+cx];
            pixel = getcolor(fb, p.r, p.g, p.b);
            
            if (x+cx < 0 || y+cy < 0 || x+cx >= (int)fb->vinfo.xres || y+cy >= (int)fb->vinfo.yres || p.a <= 0) {
                continue;
            } else {
                location = (x + cx + fb->vinfo.xoffset) * (fb->vinfo.bits_per_pixel/8) + (y + cy + fb->vinfo.yoffset) * fb->finfo.line_length;
                switch (fb->vinfo.bits_per_pixel) {
                    case 16:
                        *(uint16_t*)(fb->data + location) = pixel;
                        break;
                    case 32:
                        *(uint32_t*)(fb->data + location) = pixel;
                        break;
                }
            }
        }
    }

    lua_pushnumber(L, 0);
    return 1;
}

static int lfb_drawbuffer_draw_to_drawbuffer(lua_State *L) {
    drawbuffer_t *origin_db = (drawbuffer_t *)lua_touserdata(L, 1);
    drawbuffer_t *target_db = (drawbuffer_t *)lua_touserdata(L, 2);
    
    int target_x = lua_tointeger(L, 3);
    int target_y = lua_tointeger(L, 4);
    
    int origin_x = lua_tointeger(L, 3);
    int origin_y = lua_tointeger(L, 4);
    
    int w = lua_tointeger(L, 5);
    int h = lua_tointeger(L, 6);
    
    int cx;
    int cy;

    for (cy=origin_x; cy < origin_x+h; cy=cy+1) {
        for (cx=origin_x; cx < origin_x+w; cx=cx+1) {
            if (target_x + cx >= target_db->w || target_y + cy >= target_db->h || origin_x + cx >= origin_db->w || origin_y + cy >= origin_db->h ) {
                continue;
            } else {
                target_db->data[(cy+target_y)*target_db->w+cx+target_x] = origin_db->data[cy*origin_db->w+cx];
            }
        }
    }

    lua_pushnumber(L, 0);
    return 1;
}

static int lfb_drawbuffer_get_pixel(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    
    pixel_t p = db->data[y*db->w+x];
    
    lua_pushinteger(L, p.r);
    lua_pushinteger(L, p.g);
    lua_pushinteger(L, p.b);
    lua_pushinteger(L, p.a);
    
    return 4;
}

static int lfb_drawbuffer_set_pixel(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int r = lua_tointeger(L, 4);
    int g = lua_tointeger(L, 5);
    int b = lua_tointeger(L, 6);
    int a = lua_tointeger(L, 7);
    
    pixel_t p = {.r=r, .g=g, .b=b, .a=a};
    
    db->data[y*db->w+x] = p;
    
    return 0;
}

static int lfb_drawbuffer_set_rect(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int w = lua_tointeger(L, 4);
    int h = lua_tointeger(L, 5);
    int r = lua_tointeger(L, 6);
    int g = lua_tointeger(L, 7);
    int b = lua_tointeger(L, 8);
    int a = lua_tointeger(L, 9);
    
    int cx;
    int cy;
    
    pixel_t p = {.r=r, .g=g, .b=b, .a=a};
    
    for (cy=y; cy < y+h; cy=cy+1) {
        for (cx=x; cx < x+w; cx=cx+1) {
            db->data[cy*db->w+cx] = p;
        }
    }
    
    return 0;
}

static int lfb_drawbuffer_set_box(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int w = lua_tointeger(L, 4);
    int h = lua_tointeger(L, 5);
    int r = lua_tointeger(L, 6);
    int g = lua_tointeger(L, 7);
    int b = lua_tointeger(L, 8);
    int a = lua_tointeger(L, 9);
    
    int cx;
    int cy;
    
    pixel_t p = {.r=r, .g=g, .b=b, .a=a};
    
    for (cy=y; cy < y+h-1; cy=cy+1) {
        db->data[cy*db->w+x] = p;
        db->data[cy*db->w+x+w-1] = p;
    }
    for (cx=x; cx < x+w-1; cx=cx+1) {
        db->data[y*db->w+cx] = p;
        db->data[(y+h-1)*db->w+cx] = p;
    }
    
    return 0;
}

static int lfb_drawbuffer(lua_State *L) {
    drawbuffer_t *db = (drawbuffer_t *)lua_newuserdata(L, sizeof(*db));
    
    db->w = lua_tointeger(L, 1);
    db->h = lua_tointeger(L, 2);
    db->data = (pixel_t *) calloc(db->w * db->h, sizeof(pixel_t));
    
    if (db->data == NULL) {
        return 0;
    }
    
    lua_createtable(L, 0, 9);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    LUA_T_PUSH_S_N("width", db->w)
    LUA_T_PUSH_S_N("height", db->h)
    LUA_T_PUSH_S_CF("get_pixel", lfb_drawbuffer_get_pixel)
    LUA_T_PUSH_S_CF("set_pixel", lfb_drawbuffer_set_pixel)
    LUA_T_PUSH_S_CF("set_rect", lfb_drawbuffer_set_rect)
    LUA_T_PUSH_S_CF("set_box", lfb_drawbuffer_set_box)
    LUA_T_PUSH_S_CF("clear", lfb_drawbuffer_clear)
    LUA_T_PUSH_S_CF("draw_to_framebuffer", lfb_drawbuffer_draw_to_framebuffer)
    LUA_T_PUSH_S_CF("draw_to_drawbuffer", lfb_drawbuffer_draw_to_drawbuffer)
    LUA_T_PUSH_S_CF("pixel_function", lfb_drawbuffer_pixel_function)
    LUA_T_PUSH_S_CF("close", lfb_drawbuffer_close)
    LUA_T_PUSH_S_CF("__gc", lfb_drawbuffer_close)
    LUA_T_PUSH_S_CF("__tostring", lfb_drawbuffer_tostring)
    lua_setmetatable(L, -2);
    
    return 1;
}


LUALIB_API int luaopen_lfb(lua_State *L) {
    lua_newtable(L);

    LUA_T_PUSH_S_S("version", VERSION)
    LUA_T_PUSH_S_CF("new_framebuffer", lfb_framebuffer)
    LUA_T_PUSH_S_CF("new_drawbuffer", lfb_drawbuffer)

    return 1;
}

// vi: sw=4 et
