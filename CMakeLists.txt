cmake_minimum_required(VERSION 3.16)

project(cloud_client VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network Sql)

add_library(cloud_contracts STATIC
    src/network/frame.cpp
    src/network/frame.h
    src/core/types.h
    src/core/contracts.h
)
target_link_libraries(cloud_contracts PUBLIC Qt6::Network Qt6::Sql)
target_include_directories(cloud_contracts PUBLIC src)

add_executable(cloud_client
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
)

target_link_libraries(cloud_client PRIVATE Qt6::Widgets cloud_contracts)

include(CTest)
if(BUILD_TESTING)
    find_package(Qt6 REQUIRED COMPONENTS Test)
    add_executable(frame_tests tests/frame_tests.cpp)
    target_link_libraries(frame_tests PRIVATE cloud_contracts Qt6::Test)
    add_test(NAME frame_tests COMMAND frame_tests)
endif()
