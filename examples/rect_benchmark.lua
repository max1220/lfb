#!/usr/bin/env luajit
package.cpath = package.cpath .. ";../?.so"
package.path = package.path .. ";../?.lua"

local lfb = require("lfb")
local fb = lfb.new_framebuffer("/dev/fb0")
local varinfo = fb:get_varinfo()
local gettime = require("socket").gettime
math.randomseed(gettime())

local db = lfb.new_drawbuffer(varinfo.xres, varinfo.yres)

function printf(str, ...)
    print(str:format(...))
end

local begin = gettime()
local rects = 0
local maxrects = 100
local pixels = 0
local redraw = true
while true do
    local r = math.floor(math.random(0,255))
    local g = math.floor(math.random(0,255))
    local b = math.floor(math.random(0,255))
    
    local x1 = math.random(0,db.width-1)
    local y1 = math.random(0,db.height-1)
    
    local x2 = math.random(0,db.width-1)
    local y2 = math.random(0,db.height-1)
    
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
        
    db:set_rect(x1,y1, w,h, r,g,b, 255)
    
    if redraw then
	db:draw_to_framebuffer(fb, 0,0)
    end
    
    pixels = pixels + w*h
    rects = rects + 1

    if rects % maxrects == 0 then
	local dt = (gettime() - begin)
	if redraw then
	    -- print("Onscreen:  " .. maxrects .. " rects in " .. dt*1000 .. "ms, " .. rects/dt .. " rects/s, "..pixels.."px, "..pixels/dt.."px/s")
	    printf("Onscreen:  %d rects in %dms, %d rects/s, %dpx, %dpx/s", maxrects, dt*1000, rects/dt, pixels, pixels/dt)
	    redraw = false
	else
	    --print("Offscreen: " .. maxrects .. " rects in " .. dt*1000 .. "ms, " .. rects/dt .. " rects/s, "..pixels.."px, "..pixels/dt.."px/s")
	    printf("Offscreen:  %d rects in %dms, %d rects/s, %dpx, %dpx/s", maxrects, dt*1000, rects/dt, pixels, pixels/dt)
	    local start = gettime()
	    db:draw_to_framebuffer(fb, 0,0)
	    print("", "Drawing:   " .. (gettime()-start)*1000 .. "ms")
	    redraw = true
	end
	print()
	begin = gettime()
    end
end
