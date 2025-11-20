cmake_minimum_required(VERSION 3.16)
project(VideoStreaming)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add subdirectories
add_subdirectory(common)
#add_subdirectory(client)
add_subdirectory(server)
