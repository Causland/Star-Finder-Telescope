#!/bin/bash
# Run all tests registered with ctest

# Usage
usage()
{
   echo """run_tests.sh [-r] [-h]
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

build_dir=build-x86_64-ubuntu
if [[ "$rpi" == true ]]; then
   build_dir=build-armv8-rpi3
fi

if [ ! -d $build_dir ]; then
   echo "Build directory $build_dir not configured"
   exit
fi

cd $build_dir

ctest -j4 --repeat-until-fail 5 --schedule-random --output-junit test_results.xml --output-on-failure

