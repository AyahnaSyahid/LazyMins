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
    PrintSupport
    SerialPort
    Core5Compat
)

qt_standard_project_setup()

# ==================== 1. Core & Utils Library ====================
# Library untuk utilitas dasar dan koneksi database awal
add_library(core_utils STATIC
    src/utils/authmanager.cpp
    src/utils/sessionmanager.cpp
    src/utils/sqltransaction.cpp
    src/database/databasemanager.cpp
    src/database/initializeschema.cpp
)
target_link_libraries(core_utils PUBLIC Qt6::Sql Qt6::Core Qt6::Network)

# ==================== 2. Data Layer (Managers) ====================
# Berisi logika bisnis dan manajemen data 
add_library(data_layer STATIC
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
    src/managers/itemflowservice.cpp
    src/managers/financialledgerservice.cpp
    src/controllers/infopercetakan.cpp
)
target_link_libraries(data_layer PUBLIC core_utils)

# ==================== 3. UI Components (Models & Customs) ====================
# Berisi model data dan widget custom [cite: 3, 5]
add_library(model_components STATIC
    src/models/ordermodel.cpp
    src/models/advancedquerymodel.cpp
    src/models/finishinglistmodel.cpp
    src/models/priceleveleditormodel.cpp
    src/models/invoicecomposermodel.cpp
)

target_link_libraries(model_components PUBLIC data_layer)

add_library(ui_components STATIC
    src/customs/querycombobox.cpp
    src/customs/rolescombobox.cpp
    src/customs/tipekonsumencombobox.cpp
    src/customs/productcategoriescombobox.cpp
    src/customs/finishingitemdelegate.cpp
    src/customs/orderitemdelegate.cpp
    src/customs/invoicecomposerdelegate.cpp
)
target_link_libraries(ui_components PUBLIC model_components Qt6::Widgets)

# ==================== 4. Dialogs Library ====================
# Berisi semua dialog interaktif [cite: 3, 4]
add_library(ui_dialogs STATIC
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
    src/dialogs/akuntransaksiopnamedialog.cpp
    src/display/dataviewer.cpp
    src/display/finishingservicesviewer.cpp
    src/display/orderdataviewer.cpp
    src/display/produkdataviewer.cpp
    src/display/invoicedataviewer.cpp
    src/display/akuntransaksidataviewer.cpp
    src/display/konsumendataviewer.cpp
    src/display/paymentsdataviewer.cpp
    src/dialogs/depositdialog.cpp
    src/display/invoicebrowser.cpp
    src/dialogs/createfinishingservicedialog.cpp
    src/customs/buttonguard.cpp
    src/dialogs/infopercetakandialog.cpp
)

target_link_libraries(ui_dialogs PUBLIC ui_components Qt6::Widgets Qt6::Gui)
# Header tambahan jika diperlukan
# set_target_properties(ui_dialogs PROPERTIES
    # SOURCES "src/dialogs/fieldmap.h"
# )

# Modul khusus laporan
add_subdirectory(src/modul_laporan)

# ==================== 5. Printing Module ====================
# Modul khusus printer [cite: 5]
add_library(printing_module STATIC
    src/printer/posprinter.cpp
    src/printer/posprintertestdialog.cpp
    src/printer/receipt_debug.cpp
    src/printer/printservice.cpp
)
target_link_libraries(printing_module PUBLIC data_layer Qt6::PrintSupport Qt6::SerialPort EscPosQt)

# Modul khusus printer [cite: 5]
add_library(printer_setup_module STATIC
    src/dialogs/configureserialposdialog.cpp)
    
target_link_libraries(printer_setup_module PUBLIC Qt6::Widgets Qt6::PrintSupport Qt6::SerialPort EscPosQt)

enable_testing()

add_subdirectory(escpos-qt)
add_subdirectory(tests)

# ==================== Executable Utama ====================
# Menggabungkan semua library dan file utama [cite: 1, 5, 6]

set(EXECUTABLE_SOURCES
    src/main.cpp
    src/mainwindow/mainwindow.cpp
    src/mainwindow/actiongroup.cpp
    src/setup/setupwindow.cpp
    resources/database_resources.qrc
    resources/res.rc
    )

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    qt_add_executable(LazyMins WIN32 ${EXECUTABLE_SOURCES})
else()
    qt_add_executable(LazyMins ${EXECUTABLE_SOURCES})
endif()


# Link semua modul ke target utama [cite: 6]
target_link_libraries(LazyMins PRIVATE 
    core_utils
    data_layer
    ui_components
    ui_dialogs
    printing_module
    printer_setup_module
    modul_laporan
    Qt6::Widgets 
    Qt6::Sql 
    Qt6::Network
    Qt6::PrintSupport
    Qt6::SerialPort
    Qt6::Core5Compat
)

# ==================== Build Information ====================
message(STATUS "========================================")
message(STATUS "Project          : ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Build Type       : ${CMAKE_BUILD_TYPE}")
message(STATUS "Generator        : ${CMAKE_GENERATOR}")
message(STATUS "CXX Standard     : C++${CMAKE_CXX_STANDARD}")
message(STATUS "Executable       : LazyMins")
message(STATUS "========================================")