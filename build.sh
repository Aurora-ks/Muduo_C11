#! /bin/bash

set -e

if [ -d lib ]; then
    rm -r lib
fi

cmake -B build &&
    cmake --build build

rm -r build

cd lib
for lib in `ls`
do
    cp $lib /usr/lib
done