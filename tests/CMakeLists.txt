qt_add_executable(test_am 
        test_adminmanager.h
        test_adminmanager.cpp 
        )

target_include_directories(test_am PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(test_am PRIVATE Mgr_LAdmins Ui_LAdmins Qt::Test)

add_test(NAME AdminManagerTest COMMAND test_am)