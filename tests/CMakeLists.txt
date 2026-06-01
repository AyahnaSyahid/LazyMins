qt_add_executable(tstDatabase 
    ${CMAKE_CURRENT_SOURCE_DIR}/tstdatabase.cpp 
    ${PROJECT_SOURCE_DIR}/resources/database_resources.qrc
)

# 2. Include directory & Link libraries
target_include_directories(tstDatabase PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tstDatabase PRIVATE data_layer Qt6::Test)

# 3. Definisikan Test (Sudah diperbaiki dengan generator expression)
add_test(NAME DatabaseTest COMMAND $<TARGET_FILE:tstDatabase>)

# 4. Custom target untuk mempermudah build semua test
if(NOT TARGET build_tests)
    add_custom_target(build_tests)
endif()
add_dependencies(build_tests tstDatabase)