cmake_minimum_required(VERSION 3.16)
project(cnc-control-system LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS 3DCore 3DRender SerialPort Widgets)

add_executable(
    cnc-control-system src/host/main.cpp
)

target_link_libraries(cnc-control-system PRIVATE
    Qt6::Gui
    Qt6::Core
    Qt6::SerialPort
    Qt6::Widgets
)
