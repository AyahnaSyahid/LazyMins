qt_add_executable(LAdminsTests main.cpp)

target_include_directories(LAdminsTests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(LAdminsTests PRIVATE LAdmins Qt6::Sql Qt6::Widgets Qt6::Network)
