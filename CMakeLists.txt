cmake_minimum_required(VERSION 3.16)

project(HMP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


qt_add_executable(HMP
    mainprogram.cc
    mainprogram.hh
    datastructures.cc
    datastructures.hh
)

set_target_properties(HMP PROPERTIES AUTOMOC ON)
target_link_libraries(HMP PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)