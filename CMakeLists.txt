cmake_minimum_required(VERSION 3.21)

project(
    pdf-integrity-friend
    VERSION 1.0.0
    DESCRIPTION "PDF Integrity Friend by Ratio Juris"
    HOMEPAGE_URL "https://github.com"
    LANGUAGES CXX
)

# ============================================================
# Windows Only
# ============================================================

if(NOT WIN32)
    message(FATAL_ERROR
        "PDF Integrity Friend currently supports Windows only.")
endif()

# ============================================================
# Compiler Settings
# ============================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ============================================================
# Build Types
# ============================================================

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# ============================================================
# Qt
# ============================================================

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Pdf
    PdfWidgets
)

# ============================================================
# OpenSSL
# ============================================================

find_package(OpenSSL REQUIRED)

# ============================================================
# Version Definitions
# ============================================================

add_compile_definitions(
    APP_NAME="PDF Integrity Friend"
    APP_VERSION="1.0-Beta"
    APP_PUBLISHER="Ratio Juris"
)

# ============================================================
# Resources
# ============================================================

set(APP_RESOURCES
    res/resources.qrc
)

set(APP_WINDOWS_RESOURCES
    res/pdf-integrity-friend.rc
)

# ============================================================
# Sources
# ============================================================

set(APP_SOURCES
    src/main.cpp

    src/MainWindow.h
    src/MainWindow.cpp
)

# ============================================================
# Executable
# ============================================================

qt_add_executable(
    pdf-integrity-friend
    WIN32
    ${APP_SOURCES}
    ${APP_RESOURCES}
    ${APP_WINDOWS_RESOURCES}
)

# ============================================================
# Includes
# ============================================================

target_include_directories(
    pdf-integrity-friend
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

# ============================================================
# Libraries
# ============================================================

target_link_libraries(
    pdf-integrity-friend
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Pdf
        Qt6::PdfWidgets

        OpenSSL::SSL
        OpenSSL::Crypto
)

# ============================================================
# Warnings
# ============================================================

if(MSVC)

    target_compile_options(
        pdf-integrity-friend
        PRIVATE
            /W4
            /permissive-
    )

else()

    target_compile_options(
        pdf-integrity-friend
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )

endif()

# ============================================================
# Output Directories
# ============================================================

set_target_properties(
    pdf-integrity-friend
    PROPERTIES

    RUNTIME_OUTPUT_DIRECTORY
        ${CMAKE_SOURCE_DIR}/build

    ARCHIVE_OUTPUT_DIRECTORY
        ${CMAKE_SOURCE_DIR}/build

    LIBRARY_OUTPUT_DIRECTORY
        ${CMAKE_SOURCE_DIR}/build
)

# ============================================================
# Installation
# ============================================================

install(
    TARGETS pdf-integrity-friend

    RUNTIME DESTINATION .
)

install(
    FILES
        LICENSE.md
        README.md

    DESTINATION .
)

# ============================================================
# Release Folder
# ============================================================

set(
    RELEASE_DIR
    ${CMAKE_SOURCE_DIR}/release-latest
)

# ============================================================
# Information
# ============================================================

message(STATUS "")
message(STATUS "======================================")
message(STATUS "PDF Integrity Friend")
message(STATUS "Version : 1.0-Beta")
message(STATUS "Publisher : Ratio Juris")
message(STATUS "Build Type : ${CMAKE_BUILD_TYPE}")
message(STATUS "======================================")
message(STATUS "")
