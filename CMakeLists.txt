cmake_minimum_required(VERSION 3.16)

project(cloud_client VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(cloud_client
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(cloud_client PRIVATE Qt6::Widgets)
