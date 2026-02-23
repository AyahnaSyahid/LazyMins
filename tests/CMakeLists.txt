qt_add_executable(test_am 
        test_adminmanager.h
        test_adminmanager.cpp 
        )

target_include_directories(test_am PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_am PRIVATE LAdmins Qt6::Sql Qt6::Widgets Qt6::Network Qt::Test)

add_test(NAME AdminManagerTest COMMAND test_am)