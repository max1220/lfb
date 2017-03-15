#!/bin/bash

# Adjust to your needs. lua5.1 is ABI-Compatible with luajit.
CFLAGS= -O3 -Wall -Wextra -fPIC -I/usr/include/lua5.1
LIBS= -shared -llua5.1

lfb.so: lfb.c
	$(CC) -o $@ $(CFLAGS) $(LIBS) $<
	strip $@

clean:
	rm lfb.so
