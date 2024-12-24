cmake_minimum_required(VERSION 3.30)
project(hello_doom)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(3rd_party)

add_executable(hello_doom main.cpp libs.c)
#target_compile_options(hello_doom PRIVATE -Wall -Werror -Wextra)
target_link_libraries(hello_doom PRIVATE kernel32 user32 shell32 gdi32 imgui stb sokol)
add_custom_command(
        TARGET hello_doom POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/doom1.wad" "${CMAKE_CURRENT_BINARY_DIR}/doom1.wad"
        VERBATIM
)
