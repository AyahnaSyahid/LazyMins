cmake_minimum_required(VERSION 3.11)
project(LazyMins)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)

find_package(Qt5 COMPONENTS Widgets Sql Network)

# file(GLOB UI_SOURCES src/ui/files/*.ui src/ui/*.cpp src/ui/*.h src/ui/models/*.h src/ui/models/*.cpp)
# file(GLOB DB_SOURCES src/database/*.h src/database/*.cpp)

add_executable(LazyMins src/main.cpp
                        src/database/database.h
                        src/database/database.cpp
                        src/database/tablemanager.h
                        src/database/tablemanager.cpp
                        src/database/usermanager.h
                        src/database/usermanager.cpp
                        src/ui/loginform.h
                        src/ui/loginform.cpp
                        src/ui/files/loginform.ui
                        src/ui/mainwindow.h
                        src/ui/mainwindow.cpp
                        src/ui/files/mainwindow.ui
                        src/ui/createorderdialog.h
                        src/ui/createorderdialog.cpp
                        src/ui/files/createorderdialog.ui
                        src/ui/models/createordermodel.h
                        src/ui/models/createordermodel.cpp
                        src/ui/createcustomerdialog.h
                        src/ui/createcustomerdialog.cpp
                        src/ui/files/createcustomerdialog.ui
                        src/ui/createproductdialog.h
                        src/ui/createproductdialog.cpp
                        src/ui/files/createproductdialog.ui
                        src/ui/createinvoicedialog.h
                        src/ui/createinvoicedialog.cpp
                        src/ui/files/createinvoicedialog.ui
                        
                        # ${UI_SOURCES} ${DB_SOURCES}
                        resources/resources.qrc )

target_include_directories(LazyMins PUBLIC src/ui src/database)
target_link_libraries(LazyMins PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)

# test_hash
# add_executable(hashTest src/test_hash.cpp
                         # src/database/usermanager.h
                         # src/database/usermanager.cpp
                         # resources/resources.qrc )

# app configuration
target_compile_definitions(LazyMins PRIVATE APP_DATABASE_NAME="database.sqlite3")

# target_link_libraries(hashTest PRIVATE Qt5::Core Qt5::Sql)

# add_subdirectory(test_folder/DynamicDialog)

