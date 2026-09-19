# Shared resources for all test executables
qt_add_resources(TEST_RESOURCES ${CMAKE_SOURCE_DIR}/resources/database_resources.qrc)

set( SOURCES 
     ${CMAKE_CURRENT_SOURCE_DIR}/testDatabase.cpp )

qt_add_executable(tstDatabase 
    ${SOURCES}
    ${TEST_RESOURCES}
)


# 2. Include directory & Link libraries
target_include_directories(tstDatabase PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tstDatabase PRIVATE core_utils data_layer Qt6::Test)

# 3. Definisikan Test (Sudah diperbaiki dengan generator expression)
add_test(NAME DatabaseTest COMMAND $<TARGET_FILE:tstDatabase>)

# 4. Custom target untuk mempermudah build semua test
if(NOT TARGET build_tests)
    add_custom_target(build_tests)
endif()
add_dependencies(build_tests tstDatabase)

# ============================================================================
# Separate test executable for migration tests
# ============================================================================

set( MIGRATION_SOURCES 
     ${CMAKE_CURRENT_SOURCE_DIR}/testMigration.cpp )

qt_add_executable(tstMigration 
    ${MIGRATION_SOURCES}
    ${TEST_RESOURCES}
)

target_include_directories(tstMigration PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(tstMigration PRIVATE core_utils data_layer Qt6::Test)

add_test(NAME MigrationTest COMMAND $<TARGET_FILE:tstMigration>)
add_dependencies(build_tests tstMigration)