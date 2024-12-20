

set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB PVSRC ../src/database/*.cpp ../src/database/*.h)

add_library(LazyDB SHARED ${PVSRC})
target_include_directories(LazyDB PUBLIC ../src/ ../src/database/)
set_target_properties(LazyDB PROPERTIES PREFIX "" SUFFIX "-LA.dll")
target_link_libraries(LazyDB PRIVATE Qt5::Sql Qt5::Widgets)

add_executable(testuserapi testuserapi.cpp)

target_include_directories(testuserapi PUBLIC ../src/ ../src/database/)
target_link_libraries(testuserapi PRIVATE Qt5::Sql Qt5::Widgets LazyDB)