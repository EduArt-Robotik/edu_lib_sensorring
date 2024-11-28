#!/bin/bash
 
 rm build -r
 mkdir build
 cd build
 cmake ../cmake
 cmake --build . -j
