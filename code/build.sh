#!/bin/bash
mkdir -p ../build
pushd ../build
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -Wno-c++11-compat-deprecated-writable-strings  -Wno-return-type -Wno-unused-function -c ../code/pong.cpp
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -dynamiclib -o libpong.so pong.o
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -c ../code/pong_sdl.cpp
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -lSDL2 -lSDL2_Test -o pong pong_sdl.o
#rm -rf Pong.app
#mkdir -p Pong.app/Contents/MacOS
#mkdir -p Pong.app/Contents/Resources
#cp pong Pong.app/Contents/MacOS/Pong
#cp libpong.so Pong.app/Contents/MacOS/libpong.so
popd
