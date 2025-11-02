cmake_minimum_required(VERSION 3.16)
project(alda_medical)

set(CMAKE_CXX_STANDARD 20)

# Enable Qt's MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set Qt path - adjust this to your Qt installation

# For Homebrew Qt on Intel Mac:
set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt")
# Or if you installed Qt manually:
# set(CMAKE_PREFIX_PATH "~/Qt/6.5.0/macos")

# Find Qt packages
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)

# Add source files
add_executable(alda_medical
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

# Link Qt libraries
target_link_libraries(alda_medical
        Qt6::Core
        Qt6::Widgets
)