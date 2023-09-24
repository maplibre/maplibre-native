# minimal CMakeLists.txt for QMapLibreGL usage
cmake_minimum_required(VERSION 3.16)

project(
    hellomaplibre
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(APPLE) # suppress some warnings in Qt6
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGLWidgets Widgets)
qt_standard_project_setup()

find_package(QMapLibreGL REQUIRED)

add_executable(hellomaplibre main.cpp mapwindow.cpp mapwindow.hpp)

target_link_libraries(hellomaplibre PRIVATE Qt6::Core Qt6::Gui Qt6::OpenGLWidgets Qt6::Widgets QMapLibreGL)
