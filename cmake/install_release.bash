#!/bin/bash
 
./cmake/make_release.bash
sudo cmake --install ./build #--prefix /home/user/sensorring
