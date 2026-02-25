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
                       src/database/databasemanager.cpp
                       src/managers/basemanager.cpp
                       src/managers/adminmanager.cpp
                       src/managers/appsettingsmanager.cpp
                       
                       src/utils/sessionmanager.h
                       src/utils/sessionmanager.cpp
                       
                       src/dialogs/logindialog.h
                       src/dialogs/logindialog.ui
                       src/dialogs/logindialog.cpp
                       src/dialogs/edituserdialog.h
                       src/dialogs/edituserdialog.ui
                       src/dialogs/edituserdialog.cpp
                       src/dialogs/createuserdialog.h
                       src/dialogs/createuserdialog.ui
                       src/dialogs/createuserdialog.cpp
                       
                       )

target_link_libraries(LAdmins PUBLIC Qt6::Sql Qt6::Widgets Qt6::Network)
add_subdirectory(tests)

qt_add_executable(test_logform src/main.cpp)

target_link_libraries(test_logform PRIVATE LAdmins)