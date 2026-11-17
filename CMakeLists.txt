cmake_minimum_required(VERSION 3.10)
project(DoomIIRPG)
add_compile_options(-fno-delete-null-pointer-checks)

add_compile_options(-O3 -fomit-frame-pointer -DNDEBUG -fsigned-char -Wno-write-strings -D__RETROFW__)

find_package(SDL REQUIRED)
find_package(ZLIB REQUIRED)

include_directories(
    ${SDL_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS}
    "${PROJECT_SOURCE_DIR}/third_party_libs/tinygl/include"
)

set(HASH_LIBRARY_TGT_NAME Hash-Library)
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/hash-library")
add_subdirectory("${PROJECT_SOURCE_DIR}/src")
