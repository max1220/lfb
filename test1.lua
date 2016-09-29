#!/usr/bin/env lua5.1

local lfb = require("lfb")
local fb = lfb.new("/dev/fb0")
local varinfo = fb:getvarinfo()
local fixinfo = fb:getfixinfo()

print("Static info:")
for k,v in pairs(varinfo) do
    print("", k,v)
end

print("\nFixed info:")
for k,v in pairs(fixinfo) do
    print("", k,v)
end

fb:close()
