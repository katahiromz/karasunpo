del CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja . -DBUILD_SHARED_LIBS=OFF -DZLIB_INCLUDE_DIR:PATH=C:/dev/KarasunpoProject/lib/zlib-1.2.11 -DZLIB_LIBRARY_RELEASE:FILEPATH=C:/dev/KarasunpoProject/lib/zlib-1.2.11/libz.a -DJPEG_INCLUDE_DIR:PATH=C:/dev/KarasunpoProject/lib/jpeg-6b -DJPEG_LIBRARY_RELEASE:FILEPATH=C:/dev/KarasunpoProject/lib/jpeg-6b/libjpeg.a
ninja
