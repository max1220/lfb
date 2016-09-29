#!/usr/bin/env luajit
local lfb = require("lfb")
local fb = lfb.new("/dev/fb0")
local varinfo = fb:getvarinfo()
local clear = fb.clear
local setpixel = fb.setpixel

local w = varinfo.xres-1
local h = varinfo.yres-1

for c=0, 255 do
    for y=0, h do
        for x=1, w do
            setpixel(fb, x,y, c,c,c)
        end
    end
end
