cmake_minimum_required(VERSION 3.16)

project(HMP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

qt_add_executable(HMP
    main.cpp
    mainwindow.cc
    mainwindow.h
    mainwindow.ui
    mainprogram.cc
    mainprogram.hh
    datastructure.cc
    datastructure.hh
)

set_target_properties(HMP PROPERTIES AUTOMOC ON)
target_link_libraries(HMP PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)