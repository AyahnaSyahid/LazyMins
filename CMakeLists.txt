cmake_minimum_required (VERSION 3.22)

project(LazyMins_JustInvoice LANGUAGES CXX)

find_package(Qt6 COMPONENTS Widgets Sql Core5Compat SerialPort)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

add_subdirectory(external/escpos-qt)

qt_add_library(JINV STATIC 
            src/invoiceprinter.h
            src/invoiceprinter.cpp
            src/databaseinterface.h
            src/databaseinterface.cpp
            src/databaseinit.cpp
            src/widget/createinvoicedialog.h
            src/widget/createinvoicedialog.ui                
            src/widget/createinvoicedialog.cpp                
            src/widget/editinvoicedialog.h
            src/widget/editinvoicedialog.cpp
            src/widget/notacolumndelegate.h
            src/widget/notacolumndelegate.cpp
            src/widget/orderinputdialog.h
            src/widget/orderinputdialog.ui
            src/widget/orderinputdialog.cpp
            src/widget/paymentinputdialog.h
            src/widget/paymentinputdialog.ui
            src/widget/paymentinputdialog.cpp
            src/widget/repaymentinputdialog.h
            src/widget/repaymentinputdialog.ui
            src/widget/repaymentinputdialog.cpp
            src/widget/receiptpreviewdialog.h
            src/widget/jinvmainwindow.h
            src/widget/jinvmainwindow.ui
            src/widget/jinvmainwindow.cpp
            src/widget/receiptpreviewdialog.ui
            src/widget/receiptpreviewdialog.cpp
            src/widget/storeinfoeditordialog.h
            src/widget/storeinfoeditordialog.cpp
            src/widget/cashflowview.h
            src/widget/cashflowview.cpp
            src/widget/customerview.h
            src/widget/customerview.cpp
            src/widget/invoiceviews.h
            src/widget/invoiceviews.ui
            src/widget/invoiceviews.cpp
            src/widget/realtimedatawidget.h
            src/widget/paymentdataeditordialog.h
            src/widget/paymentdataeditordialog.ui
            src/widget/paymentdataeditordialog.cpp
            src/widget/pencatatpengeluaran.h
            src/widget/pencatatpengeluaran.ui
            src/widget/pencatatpengeluaran.cpp
            src/widget/advancedqueryview.cpp
            src/widget/advancedqueryview.h
            src/widget/advancedqueryview.cpp
            src/widget/konfigurasi.h
            src/widget/konfigurasi.ui
            src/widget/konfigurasi.cpp
            src/models/advancedquerymodel.h
            src/models/advancedquerymodel.cpp
            
            src/widget/summary/summarywidget.cpp
            src/widget/summary/summarywidget.h
            src/widget/summary/summarywidget.ui
            src/widget/summary/dashboardmodel.h
            src/widget/summary/dashboardmodel.cpp
            src/widget/summary/dashboarditemdelegate.h
            src/widget/summary/dashboarditemdelegate.cpp
            
            resources/dbres.qrc
            )
            
# target_include_directories(JINV PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(JINV PUBLIC Qt6::Widgets Qt6::Sql Qt6::SerialPort EscPosQt)

qt_add_executable(testJINV test/testMain.cpp)

qt_add_executable(main_app src/main.cpp)

target_link_libraries(main_app PUBLIC JINV EscPosQt)
target_include_directories(main_app PUBLIC ${CMAKE_SOURCE_DIR})

target_link_libraries(testJINV PUBLIC JINV EscPosQt)
target_include_directories(testJINV PUBLIC ${CMAKE_SOURCE_DIR})
