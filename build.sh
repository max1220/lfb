#!/bin/bash

# Adjust to your needs. lua5.1 is ABI-Compatible with luajit.
gcc -O3 -Wall -shared -fPIC `pkg-config lua5.1 --cflags` -o lfb.so lfb.c
strip lfb.so
