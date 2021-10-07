option(KARASUNPO_SHAREWARE "Make Karasunpo shareware" OFF)

# karasunpo.exe
add_executable(karasunpo WIN32 karasunpo.cpp Shareware.cpp SHA-256.cpp karasunpo_res.rc)
target_include_directories(karasunpo PRIVATE
    ${GIF_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS})
target_compile_definitions(karasunpo PRIVATE -DUNICODE -D_UNICODE)
if(KARASUNPO_SHAREWARE)
    target_compile_definitions(karasunpo PRIVATE SHAREWARE=1)
else()
    target_compile_definitions(karasunpo PRIVATE NO_SHAREWARE=1)
endif()

target_link_libraries(karasunpo PRIVATE
    comctl32
    imaio
    ${GIF_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${TIFF_LIBRARIES})
