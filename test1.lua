#!/usr/bin/env lua5.1

local lfb = require("lfb")
local fb = lfb.newfb("/dev/fb0")
local varinfo = lfb.getvarinfo(fb)
local fixinfo = lfb.getfixinfo(fb)

print("Static info:")
for k,v in pairs(varinfo) do
    print("", k,v)
end

print("\nFixed info:")
for k,v in pairs(fixinfo) do
    print("", k,v)
end

lfb.close(fb)
