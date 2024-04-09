#! /bin/bash

rm -r /usr/include/network

cd lib
for lib in `ls`
do
    rm /usr/local/lib/$lib
done