#! /bin/bash

cmake -B build
cmake --build build

if [ $# -eq 1 -a $1 == "-c" ];
then
    rm -r build
fi