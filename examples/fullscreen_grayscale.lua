#!/usr/bin/env luajit
package.cpath = package.cpath .. ";../?.so"
package.path = package.path .. ";../?.lua"

local lfb = require("lfb")
local fb = lfb.new_framebuffer("/dev/fb0")
local varinfo = fb:get_varinfo()
local db = lfb.new_drawbuffer(varinfo.xres-1, varinfo.yres-1)

for c=0, 255 do
    db:clear(c,c,c, 255)
    db:draw_to_framebuffer(fb,0,0)
end
