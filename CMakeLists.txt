cmake_minimum_required(VERSION 3.1)
project(farmhash)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -mavx -maes -O3")

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-cmake
)

find_package(NodeJS)

add_nodejs_module(${PROJECT_NAME} farmhash.cc)

