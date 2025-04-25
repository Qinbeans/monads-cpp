# Simple library with the goal of using C++23 monads and destructuring

cmake_minimum_required(VERSION 3.20)
project(monads)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# if on macos, use llvm
if(APPLE)
    set(CMAKE_CXX_COMPILER "/opt/homebrew/opt/llvm/bin/clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    link_libraries(c++ c++abi) # explicitly link libc++

    include_directories("/opt/homebrew/opt/llvm@20/include/c++/include")
    link_libraries("/opt/homebrew/opt/llvm/lib/c++/libc++.a")

    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/opt/homebrew/opt/llvm/lib/c++ -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++")
endif()

# include all .h files in src
file(GLOB_RECURSE HEADERS src/*.h)
include_directories(${HEADERS})

# example usage
add_executable(expected examples/expected.cpp)
add_executable(optional examples/optional.cpp)
add_executable(variant examples/variant.cpp)
add_executable(future examples/future.cpp)