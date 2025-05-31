add_library(MenuBarPlugin SHARED menubarmodifierinterface.h
                                 menubarplugin.h
                                 menubarplugin.cpp)

set_target_properties(MenuBarPlugin PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/plugins/)
configure_file(menubarplugin.json ${CMAKE_CURRENT_BINARY_DIR}/plugins/menubarplugin.json COPYONLY)
target_link_libraries(MenuBarPlugin PUBLIC Qt5::Widgets)

add_executable(testplugin menubarmodifierinterface.h
                          main.cpp)

target_link_libraries(testplugin PUBLIC Qt5::Widgets MenuBarPlugin)