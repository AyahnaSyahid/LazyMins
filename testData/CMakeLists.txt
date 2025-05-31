
add_executable(dumpDbTool ${CMAKE_CURRENT_SOURCE_DIR}/testBackupDatabase.cpp
                          ${CMAKE_SOURCE_DIR}/resources/resources.qrc)

target_link_libraries(dumpDbTool Qt5::Sql)
set_target_properties(dumpDbTool PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/testing)

add_subdirectory(PluginsExample)