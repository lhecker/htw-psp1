project(psp1beleg C)

cmake_minimum_required(VERSION 2.6)
find_package(PkgConfig REQUIRED)

set(CMAKE_C_FLAGS "-std=c99 -Wall -Wextra")
set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/bin)

add_subdirectory(src)
add_subdirectory(src/gtk)
