#!/bin/bash
 
./make_release.bash
cd ../build
cmake --install . --prefix /workspaces/ros1_noetic/sensorring
