#!/bin/bash
# Configure CMake based on provided options

# Usage
usage()
{
   echo """configure.sh [-r] [-b <Debug|Release>] [-a] [-h]
   where:
      -r   => Cross compile for Raspberry Pi
      -b   => Build type
      -a   => Enable static and dynamic analysis in build
      -h   => Output this usage
      """
   exit
}

rpi=false
build_type="Debug"
analysis=false

while getopts "rb:ah" arg; do
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
   a)
      analysis=true
      ;;
   *)
      usage
      ;;
   esac
done

cmake_flags=""

if [[ "$analysis" == true ]]; then
   cmake_flags="-DCODE_ANALYSIS=ON"
fi

if [[ "$rpi" == true ]]; then
   cmake . -B build-armv8-rpi3 $cmake_flags -DCMAKE_BUILD_TYPE=$build_type -DBUILD_RASPBERRYPI=ON --toolchain rpi-toolchain.cmake
else
   cmake . -B build-x86_64-ubuntu $cmake_flags -DCMAKE_BUILD_TYPE=$build_type
fi

