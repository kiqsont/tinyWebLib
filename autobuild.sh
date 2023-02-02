#!/bin/bash
asyncLogger/autobuild.sh
rm -rf build
cmake -B build
cmake --build build