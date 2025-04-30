cmake_minimum_required(VERSION 3.11)
project(LazyMins)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)

find_package(Qt5 COMPONENTS Widgets Sql Network)

add_executable(LazyMins WIN32 src/main.cpp
                        src/ui/files/loginform.ui
                        src/ui/loginform.h
                        src/ui/loginform.cpp
                        src/ui/files/adminwindow.ui
                        src/ui/adminwindow.h
                        src/ui/adminwindow.cpp
                        src/database/database.h
                        src/database/database.cpp
                        src/database/usermanager.h
                        src/database/usermanager.cpp
                        resources/resources.qrc )

target_include_directories(LazyMins PUBLIC src/ui src/database)
target_link_libraries(LazyMins PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)

# test_hash
add_executable(hashTest src/test_hash.cpp
                         src/database/usermanager.h
                         src/database/usermanager.cpp
                         resources/resources.qrc )

target_link_libraries(hashTest PRIVATE Qt5::Core Qt5::Sql)

