qt_add_executable(testLAdmins main.cpp)

target_include_directories(testLAdmins PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(testLAdmins PRIVATE LAdmins Qt6::Sql Qt6::Widgets Qt6::Network)
