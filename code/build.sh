#!/bin/bash
mkdir -p ../build
pushd ../build
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -c -std=c++11 ../code/pong_sdl.cpp
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -c -std=c++11 ../code/pong.cpp
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -lSDL2 -o pong pong_sdl.o pong.o
popd
