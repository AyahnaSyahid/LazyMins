cmake_minimum_required(VERSION 3.23)

project(Qt6_LazyMins VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network)

qt_standard_project_setup()

qt_add_executable(LAdmins src/main.cpp
                          resources/database_resources.qrc
                          src/utils/sqlschemaparser.cpp
                          src/database/databasemanager.cpp)

target_link_libraries(LAdmins PRIVATE Qt6::Sql Qt6::Widgets)
