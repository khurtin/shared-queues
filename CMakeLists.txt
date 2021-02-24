cmake_minimum_required(VERSION 3.19)
project(shared-queues LANGUAGES CXX)

add_compile_options(
    -Werror
    -Wall
    -Wextra
    -Wpedantic
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wctor-dtor-privacy
    -Wenum-compare
    -Wfloat-equal
    -Wnon-virtual-dtor
    -Wold-style-cast
    -Woverloaded-virtual
    -Wredundant-decls
    -Wsign-conversion
    -Wsign-promo
)
add_library(shared-queues INTERFACE)
target_include_directories(shared-queues INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_features(shared-queues INTERFACE cxx_std_17)
add_subdirectory(test)
