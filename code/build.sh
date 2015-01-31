#!/bin/bash
mkdir -p ../build

./makebundle.sh ../build/Pong

pushd ../build
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -Wno-c++11-compat-deprecated-writable-strings  -Wno-return-type -Wno-unused-function -c ../code/pong.cpp
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -dynamiclib -o Pong.app/Contents/MacOS/libpong.so pong.o
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -c ../code/pong_sdl.cpp
clang -g -Wall -DPONG_SLOW=1 -Wno-null-dereference -lSDL2 -lSDL2_Test -o Pong.app/Contents/MacOS/pong pong_sdl.o
popd

cp -r ../data ../build/Pong.app/Contents/Resources/
