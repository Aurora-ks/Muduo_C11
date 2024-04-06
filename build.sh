#! /bin/bash

cmake -B build
cmake --build build

if [ $# -eq 1 && $1 -eq "-c"];
then
    rm -r build
fi