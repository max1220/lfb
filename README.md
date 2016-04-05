lfb
===

Binding to Linux Framebuffers, using no external dependencys.



Install
-------

'''./build.sh'''

There is no installation, just copy the lfb.so file to where ever you need it.
You can check where lua looks for modules with: '''lua -e "print(package.cpath:gsub(';', '\n'):gsub('?', '[filename]'))"'''
If you want to build the module for soething diffrent than lua51(and luajit),
you'll also want to modify "pkg-config lua5.1 --cflags" to match your needs.



Usage
-----

'''
lfb.version()              -> Version string
lfb.newfb(fbdev)           -> Creates a new framebuffer userspace object & returns it. fbdev is a string, for example "/dev/fb0"
lfb.getvarinfo(fb)         -> Get variable info (See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L240)
lfb.getfixinfo(fb)         -> Get variable info(See http://lxr.free-electrons.com/source/include/uapi/linux/fb.h#L156)
lfb.setvarinfo(fb)         -> Sets variable info(Variable names are as in getvarinfo; You don't need to specify all of them, only the ones you want to change)
lfb.clear(fb,r,g,b)        -> Clears the framebuffer to the specific color
lfb.setpixel(fb,x,y,r,g,b) -> Sets the pixel at the given location to r,g,b
lfb.close(fb)              -> Closes the framebuffer
lfb.__tostring(fb)         -> Returns an identifier for the framebuffer device(For example: "Framebuffer: /dev/fb0 (inteldrmfb)")
lfb.__gc()                 -> Same as close
'''


example
-------
See test1.lua and test2.lua
