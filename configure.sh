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

mkdir -p build
cd build

cmake_args=""
if [[ "$rpi" == true ]]; then
   cmake_args="$cmake_args -DBUILD_RASPBERRYPI=ON"
fi

cmake $cmake_args ..
