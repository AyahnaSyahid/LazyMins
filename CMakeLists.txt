cmake_minimum_required(VERSION 3.23)

project(Qt6_LazyMins VERSION 1.0.0 LANGUAGES CXX)

# ==================== Global Settings ====================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ==================== Qt Packages ====================
find_package(Qt6 REQUIRED COMPONENTS 
    Widgets 
    Sql 
    Test 
    Network
)

qt_standard_project_setup()

# ==================== Source Organization ====================

# --- All Source Files (.cpp) ---
set(PROJECT_SOURCES
    # Main
    src/main.cpp

    # Utils
    src/utils/authmanager.cpp
    src/utils/sessionmanager.cpp
    src/utils/sqltransaction.cpp

    # Database
    src/database/databasemanager.cpp
    src/database/initializeschema.cpp

    # Managers
    src/managers/activitylogmanager.cpp
    src/managers/adminmanager.cpp
    src/managers/akuntransaksimanager.cpp
    src/managers/appsettingsmanager.cpp
    src/managers/basemanager.cpp
    src/managers/finishingservicemanager.cpp
    src/managers/invoicemanager.cpp
    src/managers/kategoritransaksimanager.cpp
    src/managers/konsumenmanager.cpp
    src/managers/orderitemfinishingmanager.cpp
    src/managers/orderitemmanager.cpp
    src/managers/ordermanager.cpp
    src/managers/paymentmanager.cpp
    src/managers/pricelevelmanager.cpp
    src/managers/productcategorymanager.cpp
    src/managers/productmanager.cpp
    src/managers/productpricemanager.cpp
    src/managers/rolesmanager.cpp
    src/managers/stockmovementmanager.cpp
    src/managers/transaksimanager.cpp
    src/managers/helpers.cpp

    # Models
    src/models/ordermodel.cpp
    src/models/advancedquerymodel.cpp
    src/models/finishinglistmodel.cpp
    src/models/priceleveleditormodel.cpp
    src/models/invoicecomposermodel.cpp

    # Dialogs
    src/dialogs/logindialog.cpp
    src/dialogs/edituserdialog.cpp
    src/dialogs/edituserlogininfodialog.cpp
    src/dialogs/createuserdialog.cpp
    src/dialogs/revokepassworddialog.cpp
    src/dialogs/formdialog.cpp
    src/dialogs/konsumendialog.cpp
    src/dialogs/productdialog.cpp
    src/dialogs/orderdialog.cpp
    src/dialogs/orderitemdialog.cpp
    src/dialogs/userdialog.cpp
    src/dialogs/customerpickerdialog.cpp
    src/dialogs/orderpickerdialog.cpp
    src/dialogs/finishingdialog.cpp
    src/dialogs/instantorderdialog.cpp
    src/dialogs/kategoriprodukdialog.cpp
    src/dialogs/stockopnamedialog.cpp
    src/dialogs/stockrefilldialog.cpp
    src/dialogs/priceleveleditordialog.cpp
    src/dialogs/priceleveldialog.cpp
    src/dialogs/producteditordialog.cpp
    src/dialogs/invoicecomposerdialog.cpp
    src/dialogs/akuntransaksidialog.cpp
    src/dialogs/paymentdialog.cpp
    src/dialogs/productpickerdialog.cpp
    src/dialogs/basepickerdialog.cpp
    src/dialogs/expensedialog.cpp

    # Display / Viewers
    src/display/dataviewer.cpp
    src/display/finishingservicesviewer.cpp
    src/display/orderdataviewer.cpp
    src/display/produkdataviewer.cpp
    src/display/invoicedataviewer.cpp
    src/display/akuntransaksidataviewer.cpp
    src/display/konsumendataviewer.cpp
    src/display/paymentsdataviewer.cpp

    # Custom Widgets
    src/customs/querycombobox.cpp
    src/customs/rolescombobox.cpp
    src/customs/tipekonsumencombobox.cpp
    src/customs/productcategoriescombobox.cpp
    src/customs/finishingitemdelegate.cpp
    src/customs/orderitemdelegate.cpp
    src/customs/invoicecomposerdelegate.cpp

    # Main Window & Setup
    src/mainwindow/mainwindow.cpp
    src/mainwindow/actiongroup.cpp
    src/setup/setupwindow.cpp
)

# --- Additional Headers (jika diperlukan) ---
set(PROJECT_HEADERS
    src/dialogs/fieldmap.h
    # Tambahkan header lain hanya jika benar-benar diperlukan
)

# --- Resources ---
set(PROJECT_RESOURCES
    resources/database_resources.qrc
)

# ==================== Executable ====================
qt_add_executable(LazyMins
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    ${PROJECT_RESOURCES}
)

# ==================== Linking ====================
target_link_libraries(LazyMins PRIVATE 
    Qt6::Widgets 
    Qt6::Sql 
    Qt6::Network
)

# ==================== Build Information ====================
message(STATUS "========================================")
message(STATUS "Project          : ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build Type       : ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator        : ${CMAKE_GENERATOR}")
message(STATUS "CXX Standard     : C++${CMAKE_CXX_STANDARD}")
message(STATUS "Executable       : LazyMins")
message(STATUS "========================================")
message(STATUS "========================================")