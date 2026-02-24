cmake_minimum_required(VERSION 3.23)

project(Qt6_LazyMins VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

enable_testing()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Test Network)

qt_standard_project_setup()

qt_add_library(LAdmins resources/database_resources.qrc
                       src/utils/authmanager.cpp
                       src/utils/sessionmanager.h
                       src/utils/sessionmanager.cpp
                       src/database/databasemanager.cpp
                       src/managers/basemanager.cpp
                       src/managers/adminmanager.cpp
                       )
                       

target_link_libraries(LAdmins PUBLIC Qt6::Sql Qt6::Widgets Qt6::Network)
add_subdirectory(tests)
