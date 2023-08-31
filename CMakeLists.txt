cmake_minimum_required(VERSION 3.16)

project(linux_programming)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-g -O0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")

set(CMAKE_C_FLAGS "-g -O0")
set(CMAKE_C_FLAGS_DEBUG "-O0")



#set output path
#print PROJECT_BINARY_DIR value
message(STATUS "PROJECT_BINARY_DIR=" ${PROJECT_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories("${PROJECT_SOURCE_DIR}")

add_subdirectory(__thread)
add_subdirectory(dup_dup2)
add_subdirectory(unix_client_server)
add_subdirectory(macro)
add_subdirectory(obtain_container_ip)
add_subdirectory(named_pipe)
add_subdirectory(ttimer)
add_subdirectory(process)
add_subdirectory(array_sizeof)