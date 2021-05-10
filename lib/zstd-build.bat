cd build\cmake
mkdir builddir
cd builddir
del CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja ..
ninja
cd ..\..\..
