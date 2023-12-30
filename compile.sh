#!/bin/bash

# sudo apt install build-essential binutils-dev git cmake genisoimage libassimp-dev 

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
    ./configure LIBS="-lobstack"
fi
sudo -E make install -j$(nproc)
popd

echo "------Compiling OpenVCL------"
pushd dependencies/openvcl
sudo -E make install -j$(nproc)
popd

# PS2Stuff
if [ ! -d "dependencies/ps2stuff" ]; then
    echo "------Cloning ps2stuff------"
    git clone https://github.com/ps2dev/ps2stuff.git dependencies/ps2stuff
fi
echo "------Compiling ps2gl------"
pushd dependencies/ps2stuff && make install -j$(nproc); popd

# PS2GL
if [ ! -d "dependencies/ps2gl" ]; then
    echo "------Cloning ps2gl------"
    git clone https://github.com/nicholas477/ps2gl.git dependencies/ps2gl
fi
echo "------Compiling ps2gl------"
pushd dependencies/ps2gl && make install -j$(nproc); popd
pushd dependencies/ps2gl/glut && make install -j$(nproc); popd

echo "------Compiling ps2-engine------"
make iso -j$(nproc)