#!/bin/bash

rm -f CMakeCache.txt
cmake -G "MSYS Makefiles" -DCMAKE_BUILD_TYPE=Release .
