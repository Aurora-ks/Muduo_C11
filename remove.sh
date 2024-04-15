#! /bin/bash

cd lib
for lib in `ls`
do
    if [ -f /usr/lib/$lib ]; then
        rm /usr/lib/$lib
    fi
done