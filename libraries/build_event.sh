#!/bin/bash

cd event
if ! [ -d build ]
then
	mkdir build
fi
cd build
cmake .. -DBUILD_TESTING=OFF -DEVENT__DISABLE_BENCHMARK=ON -DEVENT__DISABLE_SAMPLES=ON
make clean
make -j 12
