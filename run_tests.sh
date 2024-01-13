#!/bin/bash
# Run all tests registered with ctest

if [ ! -d build ]; then
   echo "Build directory not configured"
   exit
fi

cd build

ctest -j4 --repeat-until-fail 5 --schedule-random --output-junit test_results.xml

