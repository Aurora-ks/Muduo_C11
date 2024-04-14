#! /bin/bash

if [ -d /usr/include/network ]; then
    rm -r /usr/include/network
fi

cd lib
for lib in `ls`
do
    if [ -d lib ]; then
        rm /usr/local/lib/$lib
    fi
done