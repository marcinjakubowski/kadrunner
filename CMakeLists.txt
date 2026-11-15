cmake_minimum_required(VERSION 3.22)
project(kadrunner VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Qml Network)
find_package(KF6Runner REQUIRED)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6GlobalAccel REQUIRED)
find_package(LayerShellQt REQUIRED)

qt_add_executable(kadrunner
    src/main.cpp
)

qt_add_qml_module(kadrunner
    URI KadRunner
    VERSION 1.0
    QML_FILES
        src/Main.qml
)

target_link_libraries(kadrunner PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::Network
    KF6::Runner
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::GlobalAccel
    LayerShellQt::Interface
)
