#!/bin/bash
# Quick build the application from the root directory

# Usage
usage()
{
   echo """compile.sh [-r] [-t] [-d] [-h]
   where:
      -r   => Cross compile for Raspberry Pi
      -t   => Build unit tests
      -d   => Generate documentation
      -h   => Output this usage
      """
   exit
}

rpi=false
test=false
docs=false

while getopts "rtdh" arg; do
   case $arg in 
   r)
      rpi=true
      ;;
   t) 
      test=true
      ;;
   d)
      docs=true
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
if [[ "$test" == true ]]; then
   cmake_args="$cmake_args -DUNIT_TEST=ON"
fi

cmake $cmake_args ..
make

if [[ "$docs" == true ]]; then
   make docs
fi

