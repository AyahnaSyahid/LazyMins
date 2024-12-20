
# set(CMAKE_INCLUDE_CURRENT_DIR ON)
# set(CMAKE_RUNTIME_OUTPUT_DIRERCTORY ${CMAKE}

add_executable(ClientServerTest qtsocket/main.cpp
                                qtsocket/connectionhandler.cpp
                                qtsocket/myserver.cpp
                                qtsocket/runnable.cpp
                                qtsocket/comunicator.cpp
                                )

target_link_libraries(ClientServerTest PRIVATE Qt5::Network Qt5::Core)