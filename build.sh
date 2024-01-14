#!/bin/bash
# Quick build the application from the root directory

# Usage
usage()
{
   echo """build.sh [-r] [-t] [-d] [-h]
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

build_dir=build-x86_64-ubuntu
if [[ "$rpi" == true ]]; then
   build_dir=build-armv8-rpi3
fi

if [ ! -d $build_dir ]; then
   echo "Build directory $build_dir not configured"
   exit
fi

cmake --build $build_dir -- -j8

if [[ "$test" == true ]]; then
   cmake --build $build_dir -- -j8 tests
fi

if [[ "$docs" == true ]]; then
   cmake --build $build_dir -- -j8 docs
fi

