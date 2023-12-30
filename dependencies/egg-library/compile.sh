#!/bin/bash

make install -j$(nproc) && mkdir -p build && pushd build && sudo make install -j$(nproc); popd
