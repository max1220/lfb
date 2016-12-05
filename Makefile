#!/bin/bash

CFLAGS= -O3 -Wall -Wextra -fPIC `pkg-config lua5.1 --cflags`
LIBS= -shared `pkg-config lua5.1 --libs`

# Adjust to your needs. lua5.1 is ABI-Compatible with luajit.
lfb.so: lfb.c
	$(CC) -o $@ $(CFLAGS) $(LIBS) $<
	strip $@

clean:
	rm lfb.so
