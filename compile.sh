#!/bin/bash

set -e

if [ ! "$1" ] || [ $1 != "ci" ]; then
    # Check if apt is installed before trying to run prereqs
    if command -v apt 2>&1 /dev/null; then
        sudo apt install -y build-essential binutils-dev git cmake genisoimage libassimp-dev libmagick++-dev sox autoconf automake autopoint
    fi

    if command -v pacman 2>&1 /dev/null; then
        sudo pacman -S gcc make cmake patch texinfo flex bison gettext wget gsl gmp zlib mpfr mpc glibc linux-headers linux-api-headers ncurses assimp jsoncpp cdrkit libelf cimg imagemagick pkgconf sox
    fi
fi

# Meshoptimizer
if [ ! -d "dependencies/meshoptimizer" ]; then
    echo "------Cloning meshoptimizer------"
    git clone https://github.com/zeux/meshoptimizer.git dependencies/meshoptimizer
fi
echo "------Compiling meshoptimizer------"
pushd dependencies/meshoptimizer
mkdir -p build
cd build
cmake ..
sudo make install -j$(nproc)
popd

# cxxopts
if [ ! -d "dependencies/cxxopts" ]; then
    echo "------Cloning cxxopts------"
    git clone https://github.com/artpaul/cxxopts.git dependencies/cxxopts
fi
echo "------Compiling cxxopts------"
pushd dependencies/cxxopts
mkdir -p build
cd build
cmake ..
sudo make install -j$(nproc)
popd

echo "------Compiling egg-library------"
pushd dependencies/egg-library && ./compile.sh; popd
echo "------Compiling egg-ps2-graphics-library------"
pushd dependencies/egg-ps2-graphics-lib && ./compile.sh; popd
echo "------Compiling ps2-manifest-generator------"
pushd tools/ps2-manifest-generator && ./compile.sh; popd
echo "------Compiling ps2-mesh-converter------"
pushd tools/ps2-mesh-converter && ./compile.sh; popd

# OpenVCL/Masp
if [ ! -d "dependencies/openvcl" ]; then
    echo "------Cloning OpenVCL/Masp------"
    git clone https://github.com/nicholas477/openvcl.git dependencies/openvcl
fi
echo "------Compiling Masp------"
pushd dependencies/openvcl/contrib/masp
if [ ! -f "Makefile" ]; then
    chmod +x ./configure
    if [ ! -z "$1" ] && [ $1 = "ci" ]; then
        ./configure LIBS="-lobstack"
    else
        ./configure
    fi
fi
make -j$(nproc)
sudo -E make install -j$(nproc)
popd

echo "------Compiling OpenVCL------"
pushd dependencies/openvcl
sudo -E make install -j$(nproc)
popd

# PS2GDB
if [ ! -d "dependencies/ps2gdb" ]; then
    echo "------Cloning ps2gdb------"
    git clone https://github.com/ps2dev/ps2gdb.git dependencies/ps2gdb
fi
echo "------Compiling ps2gdb------"
pushd dependencies/ps2gdb && make clean && make install; popd

echo "------Compiling ps2-engine------"

if [ ! -z "$1" ] && [ $1 = "ci" ]; then
    make iso -j$(nproc)
elif [ ! "$1" ] || [ $1 != "deploy" ]; then
    make iso -j$(nproc)
else
    make deploy_iso -j$(nproc)
fi