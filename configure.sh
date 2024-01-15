#!/bin/bash
# Configure CMake based on provided options

# Usage
usage()
{
   echo """configure.sh [-r] [-b <Debug|Release>] [-h]
   where:
      -r   => Cross compile for Raspberry Pi
      -b   => Build type
      -h   => Output this usage
      """
   exit
}

rpi=false
build_type="Debug"

while getopts "rb:h" arg; do
   case $arg in 
   r)
      rpi=true
      ;;
   b)
      if [ "$OPTARG" = "Debug" ] || [ "$OPTARG" = "Release" ]; then
         build_type=$OPTARG
      else
         usage
      fi
      ;;
   *)
      usage
      ;;
   esac
done

if [[ "$rpi" == true ]]; then
   cmake . -B build-armv8-rpi3 -DCMAKE_BUILD_TYPE=$build_type -DBUILD_RASPBERRYPI=ON --toolchain rpi-toolchain.cmake 
else
   cmake . -B build-x86_64-ubuntu -DCMAKE_BUILD_TYPE=$build_type
fi

