cmake_minimum_required(VERSION 3.28)
project(CMakePngDecoder)

set(EXEC_TARGET PngDecoderApp)
file(GLOB_RECURSE HEADERS src/*.c)
file(GLOB_RECURSE SOURCES include/*.h src/*.h)

# add_library(Lib SHARED IMPORTED)
# set_property(TARGET Lib PROPERTY IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/lib/zlibwapi.dll")
# set_property(TARGET Lib PROPERTY IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/lib/zlibwapi.lib")

add_executable(${EXEC_TARGET} ${HEADERS} ${SOURCES})
target_include_directories(${EXEC_TARGET} PRIVATE include src)
#target_link_libraries(PngDecoderApp Lib)

find_package(ZLIB REQUIRED)
find_package(SDL2 REQUIRED)

#add_subdirectory(assets PngDecoderApp)

target_link_libraries(${EXEC_TARGET} PRIVATE 
  ZLIB::ZLIB
  SDL2::SDL2
)

set(ResourcesPath ${CMAKE_CURRENT_SOURCE_DIR}/resources)
add_custom_command(TARGET ${EXEC_TARGET} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "Copy Resources..."
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${ResourcesPath} $<TARGET_FILE_DIR:${EXEC_TARGET}>/resources
)