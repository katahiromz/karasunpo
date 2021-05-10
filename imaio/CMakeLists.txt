# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

# enable testing
enable_testing()

# project name and language
project(imaio C)

##############################################################################

include_directories(.)

find_package(GIF)
if (GIF_FOUND)
    add_definitions(-DHAVE_GIF)
endif()

find_package(JPEG)
if (JPEG_FOUND)
    add_definitions(-DHAVE_JPEG)
endif()

find_package(PNG)
if (PNG_FOUND)
    add_definitions(-DHAVE_PNG)
endif()
add_definitions(${PNG_DEFINITIONS})

find_package(TIFF)
if (TIFF_FOUND)
    add_definitions(-DHAVE_TIFF)
endif()

# add static library
add_library(imaio STATIC
    src/imaio.c
    src/gif.c
    src/jpg.c
    src/png.c
    src/tif.c)

target_include_directories(imaio PRIVATE
    ${GIF_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS})

target_link_libraries(imaio PRIVATE
    ${GIF_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${TIFF_LIBRARIES})

# add shared library
add_library(imaio-dll SHARED
    src/imaio.c
    src/gif.c
    src/jpg.c
    src/png.c
    src/tif.c)
set_target_properties(imaio-dll PROPERTIES OUTPUT_NAME "imaio")

target_compile_definitions(imaio-dll PRIVATE -DIMAIO_DLL)

target_include_directories(imaio-dll PRIVATE
    ${GIF_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS})

target_link_libraries(imaio-dll PRIVATE
    ${GIF_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${TIFF_LIBRARIES})

# add test program #1
add_executable(imaio_test_1 tests/imaio_test.c)
target_link_libraries(imaio_test_1 PRIVATE imaio)

# add test program #2
add_executable(imaio_test_2 tests/imaio_test.c)
target_link_libraries(imaio_test_2 PRIVATE imaio-dll)
target_compile_definitions(imaio_test_2 PRIVATE -DIMAIO_DLL)

## add test
#add_test(NAME imaio_test_1 COMMAND imaio_test_1 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
#add_test(NAME imaio_test_2 COMMAND imaio_test_2 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

##############################################################################
