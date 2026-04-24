qt_add_executable(tstDatabase tstDatabase.cpp ${PROJECT_SOURCE_DIR}/resources/database_resources.qrc)

target_include_directories(tstDatabase PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tstDatabase PRIVATE data_layer Qt6::Test)

add_test(NAME DatabaseTest COMMAND tstDatabase)