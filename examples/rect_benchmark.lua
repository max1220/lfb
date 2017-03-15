#!/usr/bin/env luajit
local lfb = require("lfb")
local fb = lfb.new_fb("/dev/fb0")
local varinfo = fb:get_varinfo()
local gettime = require("socket").gettime
math.randomseed(gettime())

local buffer = lfb.new_drawbuffer(varinfo.xres, varinfo.yres)

local begin = gettime()
local rects = 0
local maxrects = 100
local pixels = 0
local redraw = true
while true do
    local r = math.floor(math.random(0,255))
    local g = math.floor(math.random(0,255))
    local b = math.floor(math.random(0,255))
    
    local x1 = math.random(0,varinfo.xres-1)
    local y1 = math.random(0,varinfo.yres-1)
    
    local x2 = math.random(0,varinfo.xres-1)
    local y2 = math.random(0,varinfo.yres-1)
    
    if x2 < x1 then
	local s = x1
	x1 = x2
	x2 = s
    end
    
    if y2 < y1 then
	local s = y1
	y1 = y2
	y2 = s
    end
    
    local w = x2-x1
    local h = y2-y1
        
    buffer:set_rect(x1,y1, w,h, r,g,b)
    
    if redraw then
	local ok, err = buffer:draw_to_fb(fb, 0,0)
	if not ok then
	    print("Can't draw, error: ", tostring(err))
	end
    end
    
    pixels = pixels + w*h
    rects = rects + 1

    if rects % maxrects == 0 then
	local dt = (gettime() - begin)
	if redraw then
	    print("Onscreen:  " .. maxrects .. " rects in " .. dt*1000 .. "ms, " .. rects/dt .. " rects/s, "..pixels.."px, "..pixels/dt.."px/s")
	    redraw = false
	else
	    print("Offscreen: " .. maxrects .. " rects in " .. dt*1000 .. "ms, " .. rects/dt .. " rects/s, "..pixels.."px, "..pixels/dt.."px/s")
	    local start = gettime()
	    buffer:draw_to_fb(fb, 0,0)
	    print("Drawing:   " .. (gettime()-start)*1000 .. "ms")
	    redraw = true
	end
	print()
	begin = gettime()
    end
end
