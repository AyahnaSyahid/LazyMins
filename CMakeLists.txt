cmake_minimum_required (VERSION 3.22)

project(LazyMins_JustInvoice LANGUAGES CXX)

find_package(Qt6 COMPONENTS Widgets Sql)

qt_standard_project_setup()

qt_add_library(JINV STATIC 
            src/widget/createinvoicedialog.h
            src/widget/createinvoicedialog.ui                
            src/widget/createinvoicedialog.cpp                
            src/widget/notacolumndelegate.h
            src/widget/notacolumndelegate.cpp
            src/widget/orderinputdialog.h
            src/widget/orderinputdialog.ui
            src/widget/orderinputdialog.cpp
            )
            
target_link_libraries(JINV PUBLIC Qt6::Widgets Qt6::Sql)

qt_add_executable(testJINV test/testMain.cpp)

target_link_libraries(testJINV PUBLIC JINV)

target_include_directories(testJINV PUBLIC ${CMAKE_SOURCE_DIR})