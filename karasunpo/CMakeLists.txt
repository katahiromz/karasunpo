option(KARASUNPO_CONSOLE "Make Karasunpo console app" OFF)

# karasunpo.exe
if(KARASUNPO_CONSOLE)
    add_executable(karasunpo karasunpo.cpp karasunpo_res.rc)
    target_compile_definitions(karasunpo PRIVATE KARASUNPO_CONSOLE=1)
else()
    add_executable(karasunpo WIN32 karasunpo.cpp karasunpo_res.rc)
endif()
target_include_directories(karasunpo PRIVATE
    ${GIF_INCLUDE_DIR}
    ${JPEG_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS})
target_compile_definitions(karasunpo PRIVATE -DUNICODE -D_UNICODE)

target_link_libraries(karasunpo PRIVATE
    comctl32
    imaio
    ${GIF_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${PNG_LIBRARIES}
    ${TIFF_LIBRARIES})
