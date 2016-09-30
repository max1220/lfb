#!/usr/bin/env luajit
local lfb = require("lfb")
local fb = lfb.new("/dev/fb0")
local varinfo = fb:getvarinfo()
local gettime = require("socket").gettime
math.randomseed(gettime())

local ot = gettime()
local i, p = 0, 0
while true do
    local x1, x2 = math.floor(math.random()*varinfo.xres), math.floor(math.random()*varinfo.xres)
    local y1, y2 = math.floor(math.random()*varinfo.yres), math.floor(math.random()*varinfo.yres)
    if x1 > x2 then x1, x2 = x2, x1 end
    if y1 > y2 then y1, y2 = y2, y1 end
    local r, g, b = math.floor(math.random()*256), math.floor(math.random()*256), math.floor(math.random()*256)
    fb:setrect(lfb.rect(x1, y1, x2 - x1, y2 - y1), r, g, b)
    i = i + 1
    p = p + (x2 - x1)*(y2 - y1)
    if i == 10000 then
	local t = gettime()
	print((i/(t-ot))..' rects/sec, '..(p/(t-ot))..' pixels/sec.')
	i, p = 0, 0
	ot = t
    end
end
