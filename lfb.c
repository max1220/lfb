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



#define VERSION "0.3"

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



static int lfb_close(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);

    if (lfb->fd >= 0) {
        close(lfb->fd);
        lfb->fd = -1;
        free(lfb->fbdev);
    }

    return 0;
}



static inline uint32_t getcolor(framebuffer_t * lfb, uint8_t r, uint8_t g, uint8_t b) {
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



static int lfb_clear(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);

    unsigned int x;
    unsigned int y;
    uint8_t r = lua_tointeger(L, 1);
    uint8_t g = lua_tointeger(L, 2);
    uint8_t b = lua_tointeger(L, 3);
    uint32_t color = getcolor(lfb, r, g, b);

    for (y = 0; y != lfb->vinfo.yres; y++) {
        uint8_t * data = lfb->data + (y + lfb->vinfo.yoffset) * lfb->finfo.line_length;
        switch (lfb->vinfo.bits_per_pixel) {
            case 16:
                for (x = 0; x != lfb->vinfo.xres; x++) {
                    ((uint16_t*)data)[x] = color;
                }
                break;
            case 32:
                for (x = 0; x != lfb->vinfo.xres; x++) {
                    ((uint32_t*)data)[x] = color;
                }
                break;
        }
    }

    return 0;
}



static int lfb_setpixel(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);
    int x = lua_tointeger(L, 2);
    int y = lua_tointeger(L, 3);
    int location = (x + lfb->vinfo.xoffset) * (lfb->vinfo.bits_per_pixel/8) + (y + lfb->vinfo.yoffset) * lfb->finfo.line_length;
    if (x < 0 || y < 0 || x >= (int)lfb->vinfo.xres || y >= (int)lfb->vinfo.yres) {
        return 0;
    }

    uint8_t r = lua_tointeger(L, 4);
    uint8_t g = lua_tointeger(L, 5);
    uint8_t b = lua_tointeger(L, 6);
    uint32_t pixel = getcolor(lfb, r, g, b);
    switch (lfb->vinfo.bits_per_pixel) {
        case 16:
            *(uint16_t*)(lfb->data + location) = pixel;
            break;
        case 32:
            *(uint32_t*)(lfb->data + location) = pixel;
            break;
    }

    return 0;
}



static int lfb_getvarinfo(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);

    ioctl(lfb->fd, FBIOGET_VSCREENINFO, &lfb->vinfo);

    lua_newtable(L);

    LUA_T_PUSH_S_N("xres", lfb->vinfo.xres);
    LUA_T_PUSH_S_N("yres", lfb->vinfo.yres);
    LUA_T_PUSH_S_N("xres_virtual", lfb->vinfo.xres_virtual);
    LUA_T_PUSH_S_N("yres_virtual", lfb->vinfo.yres_virtual);
    LUA_T_PUSH_S_N("xoffset", lfb->vinfo.xoffset);
    LUA_T_PUSH_S_N("yoffset", lfb->vinfo.yoffset);
    LUA_T_PUSH_S_N("bits_per_pixel", lfb->vinfo.bits_per_pixel);
    LUA_T_PUSH_S_N("grayscale", lfb->vinfo.grayscale);
    LUA_T_PUSH_S_N("nonstd", lfb->vinfo.nonstd);
    LUA_T_PUSH_S_N("activate", lfb->vinfo.activate);
    LUA_T_PUSH_S_N("width", lfb->vinfo.width);
    LUA_T_PUSH_S_N("height", lfb->vinfo.height);

    LUA_T_PUSH_S_N("pixclock", lfb->vinfo.pixclock);
    LUA_T_PUSH_S_N("left_margin", lfb->vinfo.left_margin);
    LUA_T_PUSH_S_N("right_margin", lfb->vinfo.right_margin);
    LUA_T_PUSH_S_N("upper_margin", lfb->vinfo.upper_margin);
    LUA_T_PUSH_S_N("lower_margin", lfb->vinfo.lower_margin);
    LUA_T_PUSH_S_N("hsync_len", lfb->vinfo.hsync_len);
    LUA_T_PUSH_S_N("vsync_len", lfb->vinfo.vsync_len);
    LUA_T_PUSH_S_N("sync", lfb->vinfo.sync);
    LUA_T_PUSH_S_N("vmode", lfb->vinfo.vmode);
    LUA_T_PUSH_S_N("rotate", lfb->vinfo.rotate);
    LUA_T_PUSH_S_N("colorspace", lfb->vinfo.colorspace);

    return 1;
}



static int lfb_getfixinfo(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);

    lua_newtable(L);

    LUA_T_PUSH_S_S("id", lfb->finfo.id)
    LUA_T_PUSH_S_N("smem_start", lfb->finfo.smem_start);
    LUA_T_PUSH_S_N("type", lfb->finfo.type);
    LUA_T_PUSH_S_N("type_aux", lfb->finfo.type_aux);
    LUA_T_PUSH_S_N("visual", lfb->finfo.visual);
    LUA_T_PUSH_S_N("xpanstep", lfb->finfo.xpanstep);
    LUA_T_PUSH_S_N("ypanstep", lfb->finfo.ypanstep);
    LUA_T_PUSH_S_N("ywrapstep", lfb->finfo.ywrapstep);
    LUA_T_PUSH_S_N("line_length", lfb->finfo.line_length);
    LUA_T_PUSH_S_N("mmio_start", lfb->finfo.mmio_start);
    LUA_T_PUSH_S_N("mmio_len", lfb->finfo.mmio_len);
    LUA_T_PUSH_S_N("accel", lfb->finfo.accel);
    LUA_T_PUSH_S_N("capabilities", lfb->finfo.capabilities);

    return 1;
}



static void getnumfield32(lua_State *L, const char *key, uint32_t *dest) {
    lua_pushstring(L, key);
    lua_gettable(L, -2);
    if (lua_isnumber(L, -1)) {
        *dest = lua_tonumber(L, -1);
    }
    lua_pop(L, 1);
}



static int lfb_setvarinfo(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);

    ioctl(lfb->fd, FBIOGET_VSCREENINFO, &lfb->vinfo);

    getnumfield32(L, "xres", &lfb->vinfo.xres);
    getnumfield32(L, "yres", &lfb->vinfo.yres);
    getnumfield32(L, "xres_virtual", &lfb->vinfo.xres_virtual);
    getnumfield32(L, "yres_virtual", &lfb->vinfo.yres_virtual);
    getnumfield32(L, "xoffset", &lfb->vinfo.xoffset);
    getnumfield32(L, "yoffset", &lfb->vinfo.yoffset);
    getnumfield32(L, "bits_per_pixel", &lfb->vinfo.bits_per_pixel);
    getnumfield32(L, "grayscale", &lfb->vinfo.grayscale);
    getnumfield32(L, "nonstd", &lfb->vinfo.nonstd);
    getnumfield32(L, "activate", &lfb->vinfo.activate);
    getnumfield32(L, "width", &lfb->vinfo.width);
    getnumfield32(L, "height", &lfb->vinfo.height);

    getnumfield32(L, "pixclock", &lfb->vinfo.pixclock);
    getnumfield32(L, "left_margin", &lfb->vinfo.left_margin);
    getnumfield32(L, "right_margin", &lfb->vinfo.right_margin);
    getnumfield32(L, "upper_margin", &lfb->vinfo.upper_margin);
    getnumfield32(L, "lower_margin", &lfb->vinfo.lower_margin);
    getnumfield32(L, "hsync_len", &lfb->vinfo.hsync_len);
    getnumfield32(L, "vsync_len", &lfb->vinfo.vsync_len);
    getnumfield32(L, "sync", &lfb->vinfo.sync);
    getnumfield32(L, "vmode", &lfb->vinfo.vmode);
    getnumfield32(L, "rotate", &lfb->vinfo.rotate);
    getnumfield32(L, "colorspace", &lfb->vinfo.colorspace);

    return 1;
}



static int lfb_tostring(lua_State *L) {
    framebuffer_t *lfb = (framebuffer_t *)lua_touserdata(L, 1);

    if (lfb->fbdev) {
        lua_pushfstring(L, "Framebuffer: %s (%s)", lfb->fbdev, lfb->finfo.id);
    } else {
        lua_pushfstring(L, "Closed framebuffer");
    }

    return 1;
}



static int lfb_new(lua_State *L) {
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

    lua_createtable(L, 0, 8);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    
    LUA_T_PUSH_S_CF("setpixel", &lfb_setpixel);
    LUA_T_PUSH_S_CF("clear", lfb_clear)
    LUA_T_PUSH_S_CF("close", lfb_close)
    LUA_T_PUSH_S_CF("getfixinfo", lfb_getfixinfo)
    LUA_T_PUSH_S_CF("getvarinfo", lfb_getvarinfo)
    LUA_T_PUSH_S_CF("setvarinfo", lfb_setvarinfo)
    LUA_T_PUSH_S_CF("__gc", lfb_close)
    LUA_T_PUSH_S_CF("__tostring", lfb_tostring)
    lua_setmetatable(L, -2);

    return 1;
}



LUALIB_API int luaopen_lfb(lua_State *L) {
    lua_newtable(L);

    LUA_T_PUSH_S_S("version", VERSION)
    LUA_T_PUSH_S_CF("new", lfb_new)

    return 1;
}

// vi: sw=4 et
