#!/bin/bash

cd event
if ! [ -d build ]
then
	mkdir build
fi
cd build
cmake .. -DBUILD_TESTING=OFF -DEVENT__DISABLE_BENCHMARK=ON -DEVENT__DISABLE_SAMPLES=ON -DCMAKE_C_FLAGS="-fPIC" -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_OPENSSL=ON
make clean
make -j 12
