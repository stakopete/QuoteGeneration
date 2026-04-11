cmake_minimum_required(VERSION 3.19)

project(QuoteGeneration VERSION 1.0.0 LANGUAGES CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
# Qt 6 requires C++17 as a minimum.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Qt automation ─────────────────────────────────────────────────────────────
# AUTOMOC = processes Q_OBJECT macros (signals and slots)
# AUTORCC = processes .qrc resource files
# AUTOUIC = processes .ui designer files
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Find Qt modules ───────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS
    Widgets
    Sql
    PrintSupport
    Network
    Gui
)

# ── Source files ──────────────────────────────────────────────────────────────
# Only list files that currently exist on disk.
# We will add database.cpp and database.h in the next step.
set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    database.h
    database.cpp
    configdialog.h
    configdialog.cpp
    titlesection.h
    titlesection.cpp
    animatedbutton.h
    animatedbutton.cpp
    systemsection.h
    systemsection.cpp
)

# ── Build the executable ──────────────────────────────────────────────────────
qt_add_executable(QuoteGeneration
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    systemsection.h
    systemsection.cpp
)

# ── Link Qt into the executable ───────────────────────────────────────────────
target_link_libraries(QuoteGeneration PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Network
    Qt6::Gui
)

# ── Suppress console window on Windows ───────────────────────────────────────
if(WIN32)
    set_target_properties(QuoteGeneration PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ── Required finalisation step for Qt 6 ──────────────────────────────────────
qt_finalize_executable(QuoteGeneration)