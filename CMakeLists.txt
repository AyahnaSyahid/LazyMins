cmake_minimum_required(VERSION 3.23)

project(Qt6_LazyMins VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Network)
qt_standard_project_setup()

# qt_add_executable(LMInstaller )
