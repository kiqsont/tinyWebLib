#!/bin/bash
rm -rf build 
rm -rf asyncLogger/build
rm lib/*
rm example/bin/*

cmake asyncLogger/ -B asyncLogger/build/
make --directory=asyncLogger/build/

cmake -B build
cmake --build build

cp asyncLogger/lib/libasyncLogger.so lib/
