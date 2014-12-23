#!/bin/bash
mkdir -p ../build
pushd ../build
clang -g -Wall -c -std=c++11 ../code/pong_sdl.cpp
clang -g -Wall -lSDL2 -o pong pong_sdl.o
popd
