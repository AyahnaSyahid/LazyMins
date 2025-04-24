cmake_minimum_required(VERSION 3.11)
project(LazyMins)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Sql Network)

add_executable(LazyMins src/main.cpp
                        src/ui/adminwindow.cpp
                        src/ui/adminwindow.h
                        src/ui/files/adminwindow.ui
                        resources/resources.qrc )

target_include_directories(LazyMins PUBLIC src/ui)
target_link_libraries(LazyMins PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)