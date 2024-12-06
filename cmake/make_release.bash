#!/bin/bash
 
 rm build -r
 mkdir build
 cd build
 cmake .. -DBUILD_SHARED_LIBS=ON -DVERBOSE=OFF -DCMAKE_BUILD_TYPE=Release
 cmake --build . -j4
 cpack
