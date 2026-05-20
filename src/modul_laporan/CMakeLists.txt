set(MODUL_LAPORAN_SOURCES 
    daily_expense_renderer.h
    daily_sales_renderer.h
    reportloader.cpp
    reportview.cpp
    reportdialog.cpp)

qt_add_library(modul_laporan_loader STATIC reportloader.cpp)

target_link_libraries(modul_laporan_loader PUBLIC data_layer modul_laporan Qt6::Sql)

target_include_directories(modul_laporan_loader PUBLIC ${PROJECT_SOURCE_DIR}/src/managers)

qt_add_library(modul_laporan STATIC
    ${MODUL_LAPORAN_SOURCES}
)

target_link_libraries(modul_laporan PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Sql
    Qt::PrintSupport
)

add_executable(ReportViewer WIN32 main_example.cpp)
target_link_libraries(ReportViewer PRIVATE modul_laporan Qt6::Widgets)