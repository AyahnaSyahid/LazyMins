cmake_minimum_required(VERSION 3.18)
project(LazyAdmin)
set(CMAKE_LANGUAGES CXX)

# set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(_APP_NAME_ LazyAdmin)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# LINUX
# find_package(Qt5 REQUIRED COMPONENTS Widgets)

# Windows A3
#set(Qt6_DIR C:/Qt/5.12.12/mingw73_64/lib/cmake/Qt6)
# Windows IRWAN
# set(Qt6_DIR D:/HyStack/QtFrameworks/5.12.12/mingw73_64/lib/cmake/Qt6)

find_package(Qt5 REQUIRED COMPONENTS Widgets Sql Network)

file(GLOB HDRS src/*.h)
file(GLOB SRCS src/cpp/*.cpp)
file(GLOB UIS ui/*.ui)
file(GLOB MODELS src/models/*.h src/models/cpp/*.cpp)
file(GLOB DATABASE src/database/*.h src/database/*.cpp)

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

add_executable(LazyAdmin ${HDRS} ${SRCS} ${UIS} ${MODELS} ${DATABASE} ui/lazymins.qrc ui/res.rc)
# add_executable(LazyAdmin WIN32 ${HDRS} ${SRCS} ${UIS} ${MODELS} src/ui/lazymins.qrc)

target_include_directories(LazyAdmin PUBLIC src ui src/models src/database/)
target_link_libraries(LazyAdmin PUBLIC Qt5::Widgets Qt5::Sql Qt5::Network)

# add dummy 
target_compile_definitions(LazyAdmin PRIVATE DEV_MODE_ON)

add_custom_target("Copy-Sample-Database" ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/sample.db)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sample.db
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/database/sample_src.db
                                                    ${CMAKE_CURRENT_BINARY_DIR}/sample.db
                           DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/database/sample_src.db )

# add_subdirectory(src2)
# add_subdirectory(test)

# Windows IMAH
# find_package(Qt6 REQUIRED COMPONENTS Widgets Sql)
# test 2