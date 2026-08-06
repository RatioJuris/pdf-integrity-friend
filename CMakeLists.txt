cmake_minimum_required(VERSION 3.21)

project(
    pdf-integrity-friend
    VERSION 1.0.0
    DESCRIPTION "Cross-platform PDF integrity and TSA signing panel"
    HOMEPAGE_URL "https://github.com/RatioJuris/pdf-integrity-friend"
    LANGUAGES CXX
)

# ========================================================================
# C++ Configuration
# ========================================================================

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ========================================================================
# Qt Automatic Processing
# ========================================================================

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# ========================================================================
# Project Metadata
# ========================================================================

add_compile_definitions(
    PROJECT_AUTHOR="RatioJuris"
    PROJECT_VERSION="${PROJECT_VERSION}"
)

# ========================================================================
# Dependencies
# ========================================================================

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
)

find_package(OpenSSL REQUIRED)

# ========================================================================
# Source Files
# ========================================================================

set(PROJECT_SOURCES
    main.cpp
    mainwindow.h
    mainwindow.cpp
)

# ========================================================================
# Executable
# ========================================================================

qt_add_executable(pdf-integrity-friend
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# Generate a proper macOS .app bundle
set_target_properties(pdf-integrity-friend PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.ratiojuris.pdfintegrityfriend
    MACOSX_BUNDLE_BUNDLE_NAME "PDF Integrity Friend"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    WIN32_EXECUTABLE TRUE
)

# ========================================================================
# Include Directories
# ========================================================================

target_include_directories(pdf-integrity-friend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ========================================================================
# Libraries
# ========================================================================

target_link_libraries(pdf-integrity-friend PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    OpenSSL::SSL
    OpenSSL::Crypto
)

# ========================================================================
# Compiler Warnings
# ========================================================================

if(MSVC)
    target_compile_options(pdf-integrity-friend PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(pdf-integrity-friend PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ========================================================================
# Installation
# ========================================================================

install(TARGETS pdf-integrity-friend
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# ========================================================================
# Qt Finalization
# ========================================================================

qt_finalize_executable(pdf-integrity-friend)

# ========================================================================
# Build Information
# ========================================================================

message(STATUS "-----------------------------------------")
message(STATUS "Project: ${PROJECT_NAME}")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt Version: ${Qt6_VERSION}")
message(STATUS "OpenSSL Found: ${OPENSSL_FOUND}")
message(STATUS "-----------------------------------------")
