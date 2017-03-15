lfb
===

Binding to Linux Framebuffers, using no external dependencys.



Install
-------

`make`

There is no installation, just copy the lfb.so file to where ever you need it.
You can check where lua looks for modules with: `lua -e "print(package.cpath:gsub(';', '\n'):gsub('?', '[filename]'))"`



Usage
-----

Since version 1.0 lfb uses a buffer for all drawing operations that is then drawn to the framebuffer.
Don't expect compability with versions < 1.



command                      | description
---------------------------- | -----------
lfb.version()                | Version string
fb = lfb.new_fb(fbdev)       | Creates a new framebuffer userspace object & returns it. fbdev is a string, for example "/dev/fb0"
db = lfb.new_drawbuffer(w,h) | Creates a new drawing buffer.

command               | description
--------------------- | -----------
fb:get_varinfo()      | Get variable info in a table (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L240)
fb:get_fixinfo()      | Get fixed info in a table (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L156)
fb:set_varinfo(vinfo) | Sets variable info (Variable names are as in getvarinfo; You don't need to specify all of them, only the ones you want to change)
fb:close()            | Closes the framebuffer
fb:__tostring()       | Returns an identifier for the framebuffer device (For example: "Framebuffer: /dev/fb0 (inteldrmfb)")

command                    | description
-------------------------- | -----------
r,g,b = db:get_pixel(x,y)  | Gets r,g,b values(0-255) at x,y
db:set_pixel(x,y,r,g,b)    | Sets r,g,b values(0-255) at x,y
db:set_rect(x,y,w,h,r,g,b) | Fills the rectangle specified via x,y,w,h to r,g,b(0-255)
db:set_box(x,y,w,h,r,g,b)  | Draws outline of the rectangle specified via x,y,w,h to r,g,b(0-255)
db:draw_to_fb(fb, x, y)    | Draws the drawing buffer to the framebufer at the specified coordinates




Examples
--------
See examples/ folder.
