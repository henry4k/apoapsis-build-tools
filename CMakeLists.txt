cmake_minimum_required(VERSION 2.8)

project(apoapsis-build-tools)


set(OIIO_INCLUDE_DIR NOTFOUND CACHE PATH "")
set(OIIO_LIBRARY NOTFOUND CACHE PATH "")
if(NOT OIIO_INCLUDE_DIR OR NOT OIIO_LIBRARY)
    message(FATAL_ERROR "OpenImageIO dependencies are not configured yet.")
endif()

include_directories(${OIIO_INCLUDE_DIR})

add_executable(gen-normalmap gen-normalmap.c normalmap.c image.cpp)
target_link_libraries(gen-normalmap ${OIIO_LIBRARY})
