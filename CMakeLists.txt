cmake_minimum_required(VERSION 2.8.12.2)
project(simple_web_server C)

set(CMAKE_C_STANDARD 99)

add_executable(simple_web_server main.c csapp.h poll/client.cpp poll/server.cpp)