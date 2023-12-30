#!/usr/bin/bash

mkdir -p build && pushd build && cmake .. && sudo make install -j$(nproc); popd