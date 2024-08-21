cmake_minimum_required(VERSION 3.29)
project(Q)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(deps/LLVMBrewer)

file(GLOB_RECURSE src "src/*.cpp" "include/*.hpp")
add_executable(Q ${src})
target_include_directories(Q PRIVATE "include")
target_link_libraries(Q PRIVATE brewer)
