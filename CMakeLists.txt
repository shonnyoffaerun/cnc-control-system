cmake_minimum_required(VERSION 3.16)
project(cnc-control-system LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

add_executable(
    cnc-control-system
    src/host/main.cpp
    src/host/mainwindow.cpp
    src/host/serial_bridge.cpp
    src/host/tool_manager.cpp
    src/host/visualizer.cpp
    src/host/cad_parser.cpp
    src/common/protocol.cpp
)

target_include_directories(cnc-control-system PRIVATE
    src/host
    src/common
)

target_link_libraries(cnc-control-system PRIVATE
    Qt6::Gui
    Qt6::Core
    Qt6::SerialPort
    Qt6::Widgets
)
