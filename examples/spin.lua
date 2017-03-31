#!/usr/bin/env luajit
package.cpath = package.cpath .. ";../?.so"
package.path = package.path .. ";../?.lua"
local lfb = require("lfb")
local gettime = require("socket").gettime
local fb = lfb.new_framebuffer("/dev/fb0")
local varinfo = fb:get_varinfo()
local db = lfb.new_drawbuffer(varinfo.xres, varinfo.yres)

if not arg[1] then
	print("Drawing a triangle. You can also pass a number to draw any n-edged regular geometry.")
	print("To draw a pentagon one would use:")
	print("", arg[0] .. " 5")
end

function draw_n_edged(nedges, cx, cy, rot, scale, r,g,b,a)
	local edges = {}
	local rot = tonumber(rot) or 0
	rot = math.rad(rot)
	local scale = tonumber(scale) or 10
	for i=1, nedges do
		local x = math.cos( (i/(nedges/2)) * math.pi + rot) * scale
		local y = math.sin( (i/(nedges/2)) * math.pi + rot) * scale
		table.insert(edges, {x=x, y=y})
	end

	for i=1, nedges do
		local from = edges[i]
		local to = edges[i+1] or edges[1]
		db:set_line(from.x+cx, from.y+cy, to.x+cx, to.y+cy, r,g,b,a)
	end
end


local edges = tonumber(arg[1]) or 3
local rot = 0
local size = 0
local dt = 0
while true do
	local start = gettime()

	db:clear(255,255,255,255)
	draw_n_edged(edges, db.width/2,db.height/2, rot, 50+math.sin(size)*50, 0,0,0,255)
--	fill(0,0,0,255, 80,80,80,255)
	db:draw_to_framebuffer(fb,0,0)

	dt = gettime() - start
	rot = (rot + (dt*10)) % 360
	size = (size + dt*2) % math.pi
end



