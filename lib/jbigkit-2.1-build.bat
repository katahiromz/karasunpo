cd libjbig
del CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja .
ninja
cd ..
