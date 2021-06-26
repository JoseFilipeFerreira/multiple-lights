#!/bin/sh
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../local-thrparty/install && \
make && \
./cornell