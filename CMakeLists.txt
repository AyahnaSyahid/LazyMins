cmake_minimum_required(VERSION 3.20)

project(LazyMins)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt5 COMPONENTS Widgets Test Sql Network PrintSupport REQUIRED)

add_executable(LazyAdmins src/main.cpp
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
                        src/ui/dockwidgets/dailywidget.h
                        src/ui/dockwidgets/dailywidget.cpp
                        src/ui/files/dailywidget.ui
                        src/ui/dockwidgets/customerorderswidget.h
                        src/ui/dockwidgets/customerorderswidget.cpp
                        src/ui/files/customerorderswidget.ui
                        src/ui/files/customerorderswidget.ui
                        src/ui/dockwidgets/viewer/orderviewdialog.h
                        src/ui/dockwidgets/viewer/orderviewdialog.cpp
                        src/ui/dockwidgets/viewer/invoiceviewdialog.h
                        src/ui/dockwidgets/viewer/invoiceviewdialog.cpp
                        src/ui/dockwidgets/viewer/ui_files/invoiceviewdialog.ui
                        src/ui/permissiondialog.h
                        src/ui/permissiondialog.cpp
                        src/ui/files/permissiondialog.ui
                        src/ui/passworddialogs.h
                        src/ui/passworddialogs.cpp
                        src/ui/files/changepassworddialog.ui
                        src/ui/files/userselectordialog.ui
                        src/printing/invoicegraphics.h
                        src/printing/invoicegraphics.cpp
                        src/printing/invoicemaker.h
                        src/printing/invoicemaker.cpp
                        src/printing/barcode128.cpp
                        
                        # ${UI_SOURCES} ${DB_SOURCES}
                        src/printing/inv.qrc
                        resources/resources.qrc
                        resources/res.rc)
                        
target_compile_definitions(LazyAdmins PRIVATE APP_DATABASE_NAME="database.sqlite3")
target_include_directories(LazyAdmins PUBLIC src/ui src/database src/printing)
target_link_libraries(LazyAdmins PRIVATE Qt5::Widgets Qt5::Sql Qt5::Network Qt5::PrintSupport)
# add_subdirectory(testData)
# test_hash
# add_executable(hashTest src/test_hash.cpp
                         # src/database/usermanager.h
                         # src/database/usermanager.cpp
                         # resources/resources.qrc )

# app configuration

# target_link_libraries(hashTest PRIVATE Qt5::Core Qt5::Sql)

# add_subdirectory(test_folder/DynamicDialog)

