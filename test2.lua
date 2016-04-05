#!/usr/bin/env luajit

local lfb = require("lfb")
local fb = lfb.newfb("/dev/fb0")
local varinfo = lfb.getvarinfo(fb)
local clear = lfb.clear
local setpixel = lfb.setpixel



function drawrect(sx,sy,w,h, r,g,b)
    for x=sx, sx+w do
        for y=sy, sy+h do
            setpixel(fb, x,y, r,g,b)
        end
    end
end



function renderLogo(logo, colort)
    local logo = logo or {
        "                                ",
        " r    gggg bbb                  ",
        " r    g    b  b                 ",
        " r    gggg bbb                  ",
        " R    G    B  B                 ",
        " RRRR G    BBB                  ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "                                ",
        "wwww++++rrrrggggbbbbccccppppyyyy",
        "wwww....RRRRGGGGBBBBCCCCPPPPYYYY" }
    local colort = colort or {
        [" "] = {  0,   0,   0},
        ["+"] = {192, 192, 192},
        ["."] = { 64,  64,  64},
        ["w"] = {255, 255, 255},

        ["b"] = {  0,   0, 255},
        ["B"] = {  0,   0, 192},
        ["g"] = {  0, 255,   0},
        ["G"] = {  0, 192,   0},
        ["r"] = {255,   0,   0},
        ["R"] = {192,   0,   0},

        ["c"] = {  0, 255, 255},
        ["C"] = {  0, 192, 192},
        ["p"] = {255,   0, 255},
        ["P"] = {192,   0, 192},
        ["y"] = {255, 255,   0},
        ["Y"] = {192, 192,   0} }

    local boxw = math.floor(varinfo.xres/#logo[1])
    local boxh = math.floor(varinfo.yres/#logo)
    local logow = #logo[1]
    local logoh = #logo
    for xd=1, logow do
        for yd=1, logoh do
            local color = colort[logo[yd]:sub(xd,xd)]
            if color then
                drawrect((xd-1) * boxw, (yd-1) * boxh, boxw,boxh, unpack(color))
            end
        end
    end
end

renderLogo()

io.write(string.char(27), "[1;1H") -- ANSI terminal: Cursor to top left
io.write("Press return to quit      ")
io.write(string.char(27), "[1;1H") -- ANSI terminal: Cursor to top left
io.flush()
io.read("*l")
