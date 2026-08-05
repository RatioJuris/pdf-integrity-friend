cmake_minimum_required(VERSION 3.16)
project(pdf-integrity-friend VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
find_package(OpenSSL REQUIRED)

add_executable(pdf-integrity-friend
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(pdf-integrity-friend PRIVATE 
    Qt6::Widgets 
    Qt6::Network 
    OpenSSL::SSL 
    OpenSSL::Crypto
)
