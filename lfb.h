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
