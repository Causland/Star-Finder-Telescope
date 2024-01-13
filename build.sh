#!/bin/bash
# Quick build the application from the root directory

# Usage
usage()
{
   echo """build.sh [-t] [-d] [-h]
   where:
      -t   => Build unit tests
      -d   => Generate documentation
      -h   => Output this usage
      """
   exit
}

test=false
docs=false

while getopts "tdh" arg; do
   case $arg in 
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

if [ ! -d build ]; then
   echo "Build directory not configured"
   exit
fi

cd build

make -j8

if [[ "$test" == true ]]; then
   make -j8 tests 
fi

if [[ "$docs" == true ]]; then
   make -j8 docs
fi

