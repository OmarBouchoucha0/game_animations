#!/bin/sh

gcc src/main.c -o out/app -g -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
