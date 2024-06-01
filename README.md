# Star Finder Telescope

## Overview & Design
The Star Finder Telescope (SFT) is a generic platform that can be used to command and control a custom built telescope. The SFT core library consists of subsystems that handle commands, display, movement, object tracking, and optics. Each subsystem is written with access to interfaces which can be defined by the user.

## Compile & Run

## Cross Compilation
The project currently supports cross compilation for Raspberry Pi 64bit OS; however, the user must provide the cross-compile toolchain at `~/rpi-toolchain/`. abhiTronix provides a great guide on using a pre-built toolchain that can be found [here](https://github.com/abhiTronix/raspberry-pi-cross-compilers/wiki/Cross-Compiler-CMake-Usage-Guide-with-rsynced-Raspberry-Pi-64-bit-OS#cross-compiler-cmake-usage-guide-with-rsynced-raspberry-pi-64-bit-os)

To compile with Raspberry Pi cross compilation enabled, use the following:
```
./configure.sh -r
./build.sh -r
```

[![build_test_action](https://github.com/Causland/Star-Finder-Telescope/actions/workflows/build_test.yml/badge.svg)](https://github.com/Causland/Star-Finder-Telescope/actions/workflows/build_test.yml)


