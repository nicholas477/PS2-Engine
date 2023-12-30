#!/usr/bin/bash

# sudo apt install build-essential binutils-dev git cmake genisoimage libassimp-dev 

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
    ./configure
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

echo "------Compiling ps2-engine------"
make deploy_iso -j$(nproc)