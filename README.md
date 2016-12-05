lfb
===

Binding to Linux Framebuffers, using no external dependencys.



Install
-------

`make`

There is no installation, just copy the lfb.so file to where ever you need it.
You can check where lua looks for modules with: `lua -e "print(package.cpath:gsub(';', '\n'):gsub('?', '[filename]'))"`
If you want to build the module for something diffrent than lua51(and luajit),
you'll also want to modify "pkg-config lua5.1 --cflags" to match your needs.



Usage
-----

command | description
--- | ---
lfb.version()               | Version string
fb = lfb.new(fbdev)         | Creates a new framebuffer userspace object & returns it. fbdev is a string, for example "/dev/fb0"
fb:getvarinfo()             | Get variable info in a table (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L240)
fb:getfixinfo()             | Get fixed info in a table (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L156)
fb:setvarinfo(vinfo)        | Sets variable info (Variable names are as in getvarinfo; You don't need to specify all of them, only the ones you want to change)
fb:clear(r,g,b)             | Clears the framebuffer to the specific color
fb:setpixel(x,y,r,g,b)      | Sets the pixel at the given location to r,g,b
fb:setrect(rect,r,g,b)      | Fills the pixels in the area determined by rect to r,g,b (use lfb.rect to create a rect instance)
fb:close()                  | Closes the framebuffer
fb:__tostring()             | Returns an identifier for the framebuffer device (For example: "Framebuffer: /dev/fb0 (inteldrmfb)")
lfb.rect(x,y,w,h)           | Creates a new rectangle instance; all the arguments must be non-negative integers; w,h denote the width & height respectively, in pixels, of the rectangle.


example
-------
See `test1.lua` and `test2.lua`
