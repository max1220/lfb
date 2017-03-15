#!/usr/bin/env luajit
local lfb = require("lfb")
local fb = lfb.new_fb("/dev/fb0")
local varinfo = fb:getvarinfo()
local db = lfb.new_drawbuffer(varinfo.xres-1, varinfo.yres-1)

for c=0, 255 do
    db:clear(c,c,c)
    db:draw_to_fb(fb,0,0)
end
