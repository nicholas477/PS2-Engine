#!/usr/bin/bash

make install -j$(nproc) && pushd build && sudo make install -j$(nproc); popd
