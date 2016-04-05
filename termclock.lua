#!/usr/bin/env luajit

local lfb = require("lfb")
local fb = lfb.newfb("/dev/fb0")
local sleep = require("socket").sleep
local varinfo = lfb.getvarinfo(fb)
local clear = lfb.clear
local setpixel = lfb.setpixel

--[[
    7-Segment-Style Display:

     --1--
    |     |
    2     3
    |     |
     --4--
    |     |
    5     6
    |     |
     --7--


]]

local chars = {
    [0] = { [1] = true, [2] = true, [3] = true, [5] = true, [6] = true, [7] = true },
    [1] = { [3] = true, [6] = true },
    [2] = { [1] = true, [3] = true, [4] = true, [5] = true, [7] = true },
    [3] = { [1] = true, [3] = true, [4] = true, [6] = true, [7] = true },
    [4] = { [2] = true, [3] = true, [4] = true, [6] = true },
    [5] = { [1] = true, [2] = true, [4] = true, [6] = true, [7] = true },
    [6] = { [1] = true, [2] = true, [4] = true, [5] = true, [6] = true, [7] = true },
    [7] = { [1] = true, [3] = true, [6] = true },
    [8] = { [1] = true, [2] = true, [3] = true, [4] = true, [5] = true, [6] = true, [7] = true },
    [8] = { [1] = true, [2] = true, [3] = true, [4] = true, [5] = true, [6] = true, [7] = true },
    [9] = { [1] = true, [2] = true, [3] = true, [4] = true, [6] = true, [7] = true },
}

local seginfo = {
    [1] = {  x=0,  y=0, w=8, h=2 },
    [2] = {  x=0,  y=0, w=2, h=8 },
    [3] = {  x=6,  y=0, w=2, h=8 },
    [4] = {  x=0,  y=6, w=8, h=2 },
    [5] = {  x=0,  y=6, w=2, h=8 },
    [6] = {  x=6,  y=6, w=2, h=8 },
    [7] = {  x=0, y=12, w=8, h=2 }
}


function drawrect(sx,sy,w,h)
    for x=sx, sx+w do
        for y=sy, sy+h do
            setpixel(fb, x,y, 255,63,63)
        end
    end
end


while true do
    local date = os.date("*t")

    local h1 = math.floor(date.hour / 10)
    local h2 = date.hour % 10

    local m1 = math.floor(date.min / 10)
    local m2 = date.min % 10

    local d1 = chars[h1]
    local d2 = chars[h2]
    local d3 = chars[m1]
    local d4 = chars[m2]

    for k,v in pairs(d1) do
        local cseg = seginfo[k]
        if v then
            drawrect(cseg.x, cseg.y, cseg.w, cseg.h)
        end
    end
    for k,v in pairs(d2) do
        local cseg = seginfo[k]
        if v then
            drawrect(cseg.x, cseg.y+16, cseg.w, cseg.h)
        end
    end
    drawrect(0, 32, 2, 2)
    drawrect(6, 32, 2, 2)
    for k,v in pairs(d3) do
        local cseg = seginfo[k]
        if v then
            drawrect(cseg.x, cseg.y+36, cseg.w, cseg.h)
        end
    end
    for k,v in pairs(d4) do
        local cseg = seginfo[k]
        if v then
            drawrect(cseg.x, cseg.y+52, cseg.w, cseg.h)
        end
    end

    sleep(1)
end
