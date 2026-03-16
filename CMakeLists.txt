cmake_minimum_required(VERSION 3.23)

project(Qt6_LazyMins VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

enable_testing()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Test Network)

qt_standard_project_setup()

qt_add_library(Mgr_LAdmins STATIC 
                       src/utils/authmanager.cpp
                       src/database/databasemanager.cpp
                       src/database/initializeschema.cpp
                       src/managers/basemanager.cpp
                       src/managers/adminmanager.cpp
                       src/managers/managers.cpp
                       src/managers/rolesmanager.cpp
                       src/managers/konsumenmanager.cpp
                       src/managers/appsettingsmanager.cpp
                       # src/models/orderitemeditormodel.h
                       # src/models/orderitemeditormodel.cpp
                       src/models/ordermodel.h
                       src/models/ordermodel.cpp
                       src/models/advancedquerymodel.h
                       src/models/advancedquerymodel.cpp
                       src/models/finishinglistmodel.h
                       src/models/finishinglistmodel.cpp
                       )

qt_add_library(Ui_LAdmins STATIC resources/database_resources.qrc
                           src/dialogs/logindialog.h
                           src/dialogs/logindialog.ui
                           src/dialogs/logindialog.cpp
                           src/dialogs/edituserdialog.h
                           src/dialogs/edituserlogininfodialog.h
                           src/dialogs/edituserlogininfodialog.ui
                           src/dialogs/edituserlogininfodialog.cpp
                           src/dialogs/edituserdialog.ui
                           src/dialogs/edituserdialog.cpp
                           src/dialogs/createuserdialog.h
                           src/dialogs/createuserdialog.ui
                           src/dialogs/createuserdialog.cpp
                           src/dialogs/revokepassworddialog.h
                           src/dialogs/revokepassworddialog.ui
                           src/dialogs/revokepassworddialog.cpp
                           src/dialogs/fieldmap.h
                           src/dialogs/formdialog.h
                           src/dialogs/formdialog.cpp
                           src/dialogs/konsumendialog.h
                           src/dialogs/konsumendialog.ui
                           src/dialogs/konsumendialog.cpp
                           src/dialogs/productdialog.h
                           src/dialogs/productdialog.ui
                           src/dialogs/productdialog.cpp
                           src/dialogs/orderdialog.h
                           src/dialogs/orderdialog.ui
                           src/dialogs/orderdialog.cpp
                           src/dialogs/orderitemdialog.h
                           src/dialogs/orderitemdialog.ui
                           src/dialogs/orderitemdialog.cpp
                           src/dialogs/userdialog.cpp
                           src/dialogs/userdialog.h
                           src/dialogs/userdialog.ui
                           src/dialogs/konsumenpickerdialog.h
                           src/dialogs/konsumenpickerdialog.ui
                           src/dialogs/konsumenpickerdialog.cpp
                           src/dialogs/finishingdialog.h
                           src/dialogs/finishingdialog.ui
                           src/dialogs/finishingdialog.cpp
                           
                           
                           src/display/dataviewer.h
                           src/display/dataviewer.ui
                           src/display/dataviewer.cpp
                           src/display/finishingservicesviewer.h
                           src/display/finishingservicesviewer.cpp
                           
                           src/customs/querycombobox.h
                           src/customs/querycombobox.cpp
                           src/customs/rolescombobox.h
                           src/customs/rolescombobox.cpp
                           src/customs/tipekonsumencombobox.h
                           src/customs/tipekonsumencombobox.cpp
                           src/customs/pricelevelcombobox.h
                           src/customs/productcategoriescombobox.h
                           src/customs/productcategoriescombobox.cpp
                           src/customs/flexibledelegate.h
                           src/customs/booleandelegate.h
                           
                           src/mainwindow/mainwindow.h
                           src/mainwindow/mainwindow.ui
                           src/mainwindow/mainwindow.cpp
                           
                           src/utils/sessionmanager.h
                           src/utils/sessionmanager.cpp
                           )

target_link_libraries(Mgr_LAdmins PUBLIC Qt6::Sql Qt6::Network)
target_link_libraries(Ui_LAdmins PUBLIC Mgr_LAdmins Qt6::Widgets)

add_subdirectory(tests)

qt_add_executable(test_logform src/main.cpp)

target_link_libraries(test_logform PRIVATE Mgr_LAdmins Ui_LAdmins)