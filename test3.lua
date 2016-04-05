#!/usr/bin/env luajit
local lfb = require("lfb")
local fb = lfb.newfb("/dev/fb0")
local varinfo = lfb.getvarinfo(fb)
local clear = lfb.clear
local setpixel = lfb.setpixel

local w = varinfo.xres-1
local h = varinfo.yres-1

for c=0, 255 do
    for y=0, h do
        for x=1, w do
            setpixel(fb, x,y, c,c,c)
        end
    end
end
