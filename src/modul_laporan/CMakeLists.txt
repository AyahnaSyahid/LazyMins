set(MODUL_LAPORAN_SOURCES 
    daily_expense_renderer.h
    daily_sales_renderer.h
    report_view.h)

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