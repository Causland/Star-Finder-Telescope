#!/bin/bash
# Configure CMake based on provided options

# Usage
usage()
{
   echo """configure.sh [-r] [-h]
   where:
      -r   => Cross compile for Raspberry Pi
      -h   => Output this usage
      """
   exit
}

rpi=false

while getopts "rh" arg; do
   case $arg in 
   r)
      rpi=true
      ;;
   *)
      usage
      ;;
   esac
done

cmake_args=""
if [[ "$rpi" == true ]]; then
   cmake . -B build-armv8-rpi3 -DBUILD_RASPBERRYPI=ON --toolchain rpi-toolchain.cmake 
else
   cmake . -B build-x86_64-ubuntu
fi

