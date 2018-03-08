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

lfb coordinates start at 0,0 and go to width-1,height-1. r,g,b are in the range 0-255.



command                         | description
------------------------------- | -----------
lfb.version()                   | Version string
fb = lfb.new_framebuffer(fbdev) | Creates a new framebuffer userspace object & returns it. fbdev is a string, for example "/dev/fb0"
db = lfb.new_drawbuffer(w,h)    | Creates a new drawing buffer.

command               | description
--------------------- | -----------
fb:get_varinfo()      | Get variable info in a table (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L240)
fb:get_fixinfo()      | Get fixed info in a table (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L156)
fb:set_varinfo(vinfo) | Sets variable info (Variable names are as in getvarinfo; You don't need to specify all of them, only the ones you want to change)
fb:close()            | Closes the framebuffer
fb:__tostring()       | Returns an identifier for the framebuffer device (For example: "Framebuffer: /dev/fb0 (inteldrmfb)")

command                          | description
-------------------------------- | -----------
db.width                         | Width
db.height                        | Height
r,g,b,a = db:get_pixel(x,y)      | Gets r,g,b,a at x,y
db:set_pixel(x,y,r,g,b,a)        | Sets x,y to r,g,b,a
db:set_rect(x,y,w,h,r,g,b,a)     | Fills the rectangle specified via x,y,w,h to r,g,b,a
db:set_box(x,y,w,h,r,g,b,a)      | Draws the outline of the rectangle w,h at x,y in r,g,b,a
db:set_line(x0,y0,x1,y1,r,g,b,a) | Draws a line from x0,y0 to x1,y1 in r,g,b,a
db:clear(r,g,b,a)                | Fill with r,g,b,a
db:pixel_function(function(x,y,r,g,b,a) return r,g,b,a end) | Execute a function for each pixel in db that should return the new r,g,b,a values at x,y
db:draw_to_framebuffer(fb, x, y) | Draws db to the fb at x,y
db:draw_to_drawbuffer(tdb, tx, ty, ox, oy, w, h)  | Draws db to tdb at tx,ty. ox,oy is the offset in the origin drawbuffer. w,h set the area to draw.




Examples
--------
See examples/ folder.




Screenshots
-----------

##examples/colors.lua##
![colors example](https://raw.githubusercontent.com/max1220/lfb/master/screenshots/colors.png)


##examples/spin.lua##
![spin example](https://raw.githubusercontent.com/max1220/lfb/master/screenshots/spin.png)


##examples/rect_benchmark.lua##
![rectangle benchmark example](https://raw.githubusercontent.com/max1220/lfb/master/screenshots/rect_benchmark.png)
