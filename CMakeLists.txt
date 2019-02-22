# CMakeLists.txt --- CMake project settings
#    ex) cmake -G "Visual Studio 9 2008"
#    ex) cmake -DCMAKE_BUILD_TYPE=Release -G "MSYS Makefiles"
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 2.4)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif()

# project name and language
project(karasunpo CXX)

# check build type
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# make uppercase string of build type
string(TOUPPER "${CMAKE_BUILD_TYPE}" uppercase_CMAKE_BUILD_TYPE)

# check build type again
if (NOT uppercase_CMAKE_BUILD_TYPE MATCHES "^(DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL)$")
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

## set output directory (build/)
#set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)
#set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
#set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# enable Win32 resource
enable_language(RC)

#if (WIN32)
#    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#        # using Clang
#        set(CMAKE_C_FLAGS "-static")
#        set(CMAKE_CXX_FLAGS "-static")
#    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#        # using GCC
#        set(CMAKE_C_FLAGS "-static -static-libgcc -static-libstdc++")
#        set(CMAKE_CXX_FLAGS "-static -static-libgcc -static-libstdc++")
#    elseif (MSVC)
#        # replace "/MD" with "/MT" (building without runtime DLLs)
#        set(CompilerFlags
#            CMAKE_C_FLAGS
#            CMAKE_C_FLAGS_DEBUG
#            CMAKE_C_FLAGS_RELEASE
#            CMAKE_C_FLAGS_RELWITHDEBINFO
#            CMAKE_CXX_FLAGS
#            CMAKE_CXX_FLAGS_DEBUG
#            CMAKE_CXX_FLAGS_RELEASE
#            CMAKE_CXX_FLAGS_RELWITHDEBINFO)
#        foreach(CompilerFlags ${CompilerFlags})
#            string(REPLACE "/MD" "/MT" ${CompilerFlags} "${${CompilerFlags}}")
#        endforeach()
#    endif()
#endif()

##############################################################################

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

subdirs(mstr imaio)

include_directories(. mstr imaio katahiromz_pdfium)

# executable
add_executable(karasunpo WIN32 karasunpo.cpp karasunpo_res.rc)
target_include_directories(karasunpo PRIVATE
    ${GIF_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS})
target_compile_definitions(karasunpo PRIVATE -DUNICODE -D_UNICODE)

# link
target_link_libraries(karasunpo PRIVATE
    comctl32 imaio
    ${GIF_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${TIFF_LIBRARIES})

##############################################################################
