#!/bin/bash
source ./scripts/constant.sh
echo "$CONTRACT_NAME" 
cd $CONTRACT_NAME
rm -rf build
mkdir build
cd build
cmake ..
make
cd ../..