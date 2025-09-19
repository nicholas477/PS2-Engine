#!/bin/bash

if make clean && make install -j$(nproc) && mkdir -p build && pushd build && cmake .. && sudo make install -j$(nproc); then
    popd
else
    popd
    exit 1
fi
