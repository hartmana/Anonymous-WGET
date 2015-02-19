cmake_minimum_required(VERSION 2.8.4)
project(awget)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    TCPSocket.cpp
    TCPSocket.h
    TCPServerSocket.cpp
    TCPServerSocket.h
    awget.cpp
    StringUtils.cpp
    StringUtils.h)


add_executable(awget ${SOURCE_FILES})