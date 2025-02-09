set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
cmake_minimum_required(VERSION 3.30)
project(balatro)

add_subdirectory(lib)
add_executable(balatro MACOSX_BUNDLE Balatro.cpp)
target_link_libraries(balatro lib)