# glad - OpenGL loader (generated code)
project(glad)

add_library(glad STATIC
    src/glad.c
)

target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(OpenGL REQUIRED)
target_link_libraries(glad PUBLIC OpenGL::GL)

set_target_properties(glad PROPERTIES CXX_MODULE_STD OFF)
