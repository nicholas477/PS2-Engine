#!/bin/bash

make clean && make install -j$(nproc) && mkdir -p build && pushd build && cmake .. && sudo make install -j$(nproc); popd
