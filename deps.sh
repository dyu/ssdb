#!/bin/sh

mkdir -p deps && cd deps

GIT_CLONE="git clone --depth 1 --single-branch -b"

if [ ! -e HyperLevelDB ]; then
    $GIT_CLONE master https://github.com/rescrv/HyperLevelDB.git
    cd HyperLevelDB
    mkdir build
    autoreconf -i && ./configure --prefix=$PWD/build
    cd ..
fi
