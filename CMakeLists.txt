cmake_minimum_required(VERSION 3.11)
project(LazyMins)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Sql Network)

add_executable(LazyMins src/main.cpp
                        src/cpp/adminwindow.h
                        src/cpp/adminwindow.cpp
                        src/ui-files/adminwindow.ui )

target_include_directories(LazyMins PUBLIC src src/ui-files src/cpp)
target_link_libraries(LazyMins PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)