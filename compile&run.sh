#!/bin/sh
set -eu

cmake --preset debug-asan
cmake --build --preset debug-asan --parallel
./build/cpp_bn_orderbook
