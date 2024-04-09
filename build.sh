#! /bin/bash

set -e

cmake -B build &&
    cmake --build build

rm -r build

if [ ! -d /usr/include/network ]; then
    mkdir /usr/include/network
fi

cd include
for header in `ls`
do
    cp -r $header /usr/include/network
done

cd ../lib
for lib in `ls`
do
    cp $lib /usr/local/lib
done