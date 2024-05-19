cmake_minimum_required(VERSION 3.28)
project(CMakePngDecoder)

add_library(Lib SHARED IMPORTED)
set_property(TARGET Lib PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/lib/zlibwapi.dll")
set_property(TARGET Lib PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/lib/zlibwapi.lib")

file(GLOB_RECURSE OurSources src/*.c)
file(GLOB_RECURSE OutIncludes include/*.h src/*.h)
add_executable(PngDecoderApp ${OurSources} ${OurIncludes})

target_link_libraries(PngDecoderApp Lib)

#add_subdirectory(assets PngDecoderApp)
target_include_directories(PngDecoderApp PRIVATE include src)