#!/bin/bash

original_dir=$(pwd)
cd $(dirname $0)

cd event
if ! [[ -d build ]]; then
	mkdir build
fi
cd build
cmake .. -DCMAKE_BUILD_TYPE="Debug" -DBUILD_TESTING=OFF -DEVENT__DISABLE_BENCHMARK=ON -DEVENT__DISABLE_SAMPLES=ON -DCMAKE_C_FLAGS="-fPIC" -DEVENT__DISABLE_TESTS=ON -DEVENT__DISABLE_OPENSSL=ON
[[ $? -ne 0 ]] && exit 1
make clean
[[ $? -ne 0 ]] && exit 1
make -j 12
[[ $? -ne 0 ]] && exit 1
cd ${original_dir}
