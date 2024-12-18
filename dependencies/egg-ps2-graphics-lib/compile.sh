#!/bin/bash

command -v vcl
make clean && make -j$(nproc)
