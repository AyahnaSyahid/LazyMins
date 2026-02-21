cmake_minimum_required(VERSION 3.16)
project(CardWidget VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Cari Qt6 ──────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# ── Source files ──────────────────────────────────────────────────────────
set(SOURCES
    main.cpp
    cardmodel.cpp
    carddelegate.cpp
)

set(HEADERS
    carditem.h
    cardmodel.h
    carddelegate.h
)

# ── Executable ────────────────────────────────────────────────────────────
qt_add_executable(CardWidget
    ${SOURCES}
    ${HEADERS}
)

target_link_libraries(CardWidget PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

# ── Windows: hilangkan console window di release ──────────────────────────
if(WIN32)
    set_target_properties(CardWidget PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
    )
endif()
