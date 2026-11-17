file(GLOB HEADER_FILES "*.hpp" "*.h")
file(GLOB SOURCE_FILES "*.cpp" "*.c")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(${PROJECT_NAME}
    ${SDL_LIBRARY}
    ${ZLIB_LIBRARIES}
    "${PROJECT_SOURCE_DIR}/third_party_libs/tinygl/build/src/libtinygl-static.a"
    ${HASH_LIBRARY_TGT_NAME}
)
