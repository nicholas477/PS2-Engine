# PS2-Engine
A fun little PS2 game engine i'm working on. Nothing serious

Currently supports untextured model rendering and music and that's about it.

# How to compile
1. Install PS2DEV
2. Run `sudo make install` in `dependencies/egg-library`
3. Run `sudo make install` in `tools/ps2-mesh-converter`
4. Run `make all -j$(nproc)` in the root of this repository. The iso should be in the `build` folder
