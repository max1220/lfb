#!/usr/bin/env luajit

local lfb = require("lfb")
local fb = lfb.new_fb("/dev/fb0")
local varinfo = fb:get_varinfo()
local buffer = lfb.new_drawbuffer(varinfo.xres, varinfo.yres)


local rp = { x=0.0,  y=0.0 }
local gp = { x=1.0,  y=0.0 }
local bp = { x=0.5,  y=0.866 }

local xmax = (varinfo.xres-1)/2
local ymax = (varinfo.yres-1)/2
for x=0, xmax do
    for y=0, ymax do
        local xn = (x/xmax)
        local yn = (y/ymax)

        local dist_r = math.sqrt( (rp.x-xn)^2 + (rp.y-yn)^2 )
        local dist_g = math.sqrt( (gp.x-xn)^2 + (gp.y-yn)^2 )
        local dist_b = math.sqrt( (bp.x-xn)^2 + (bp.y-yn)^2 )

        if dist_r <=1 and dist_g <=1 and dist_b <= 1 then
            -- The distance to RGB is <=1, we're in the triangle
            local r =-dist_r * 255
            local g =-dist_g * 255
            local b =-dist_b * 255
            buffer:set_pixel(x,y, r,g,b)
        end
    end
end

buffer:draw_to_fb(fb, 0, 0)



--[[

Color "Cube":

]]
