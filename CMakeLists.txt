cmake_minimum_required(VERSION 3.11)
project(LazyMins)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)

find_package(Qt5 COMPONENTS Widgets Sql Network)


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
                        src/ui/customerpickerdialog.h
                        src/ui/customerpickerdialog.cpp
                        src/ui/files/customerpickerdialog.ui
                        src/ui/productpickerdialog.h
                        src/ui/productpickerdialog.cpp
                        src/ui/files/productpickerdialog.ui
                        src/ui/createpaymentdialog.h
                        src/ui/createpaymentdialog.cpp
                        src/ui/files/createpaymentdialog.ui
                        src/ui/files/invoicesmanagerdialog.ui
                        src/ui/invoicesmanagerdialog.h
                        src/ui/invoicesmanagerdialog.cpp
                        src/ui/models/invoicesmanagermodel.h
                        src/ui/models/invoicesmanagermodel.cpp
                        src/ui/files/editorderdialog.ui
                        src/ui/editorderdialog.h
                        src/ui/editorderdialog.cpp
                        src/ui/files/createaccountdialog.ui
                        src/ui/createaccountdialog.h
                        src/ui/createaccountdialog.cpp
                        src/ui/dashboardwidget.h
                        src/ui/dashboardwidget.cpp
                        src/ui/files/dashboardwidget.ui
                        # ${UI_SOURCES} ${DB_SOURCES}
                        resources/resources.qrc )

target_compile_definitions(LazyMins PRIVATE APP_DATABASE_NAME="database.sqlite3")
target_include_directories(LazyMins PUBLIC src/ui src/database)
target_link_libraries(LazyMins PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network)


add_subdirectory(testData)
# test_hash
# add_executable(hashTest src/test_hash.cpp
                         # src/database/usermanager.h
                         # src/database/usermanager.cpp
                         # resources/resources.qrc )

# app configuration

# target_link_libraries(hashTest PRIVATE Qt5::Core Qt5::Sql)

# add_subdirectory(test_folder/DynamicDialog)

