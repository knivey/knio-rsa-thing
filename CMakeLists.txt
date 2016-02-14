project(knio-rsa-thing)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

INCLUDE_DIRECTORIES("/mpir/include/")
LINK_DIRECTORIES("/mpir/lib")

set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} mpir mpirxx)
