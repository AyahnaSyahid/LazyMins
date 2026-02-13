cmake_minimum_required (VERSION 3.22)

project(LazyMins_JustInvoice LANGUAGES CXX)

find_package(Qt6 COMPONENTS Widgets Sql Core5Compat SerialPort)

qt_standard_project_setup()

add_subdirectory(external/escpos-qt)

qt_add_library(JINV STATIC 
            src/widget/createinvoicedialog.h
            src/widget/createinvoicedialog.ui                
            src/widget/createinvoicedialog.cpp                
            src/widget/notacolumndelegate.h
            src/widget/notacolumndelegate.cpp
            src/widget/orderinputdialog.h
            src/widget/orderinputdialog.ui
            src/widget/orderinputdialog.cpp
            src/widget/paymentinputdialog.h
            src/widget/paymentinputdialog.ui
            src/widget/paymentinputdialog.cpp
            src/widget/receiptpreviewdialog.h
            src/widget/receiptpreviewdialog.ui
            src/widget/receiptpreviewdialog.cpp
            src/invoiceprinter.h
            src/invoiceprinter.cpp
            src/databaseinterface.h
            src/databaseinterface.cpp
            src/widget/customerview.h
            src/widget/customerview.cpp
            src/widget/invoiceviews.h
            src/widget/invoiceviews.ui
            src/widget/invoiceviews.cpp
            )
            
target_link_libraries(JINV PUBLIC Qt6::Widgets Qt6::Sql Qt6::SerialPort EscPosQt)

qt_add_executable(testJINV test/testMain.cpp)

target_link_libraries(testJINV PUBLIC JINV EscPosQt)

target_include_directories(testJINV PUBLIC ${CMAKE_SOURCE_DIR})
