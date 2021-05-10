# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008" .
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles" .
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.0)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

# project name and languages
project(karasunpo C CXX RC)

##############################################################################

if (MSVC)
    add_definitions(-DMSVC)
endif()

find_package(GIF REQUIRED)
if (GIF_FOUND)
    add_definitions(-DHAVE_GIF)
endif()

find_package(JPEG REQUIRED)
if (JPEG_FOUND)
    add_definitions(-DHAVE_JPEG)
endif()

find_package(PNG REQUIRED)
if (PNG_FOUND)
    add_definitions(-DHAVE_PNG)
endif()
add_definitions(${PNG_DEFINITIONS})

find_package(TIFF REQUIRED)
if (TIFF_FOUND)
    add_definitions(-DHAVE_TIFF)
endif()

include_directories(
    .
    imaio
    katahiromz_pdfium
    ${GIF_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${TIFF_INCLUDE_DIR}
)

add_subdirectory(imaio)
add_subdirectory(karasunpo)

##############################################################################
