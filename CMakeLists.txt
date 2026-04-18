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
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/database.h
    ${CMAKE_CURRENT_SOURCE_DIR}/database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/configdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/configdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stylemanager.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stylemanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/animatedbutton.h
    ${CMAKE_CURRENT_SOURCE_DIR}/animatedbutton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/titlesection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/titlesection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/systemsection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/systemsection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/basissection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/basissection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pricesection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/pricesection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/scopesection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/scopesection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exclusionssection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exclusionssection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generalconditionssection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/generalconditionssection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/clarificationssection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/clarificationssection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/signaturesection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/signaturesection.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quotepreviewdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quotepreviewdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quotetypedialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quotetypedialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dropdownmanagerdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dropdownmanagerdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/editoptiondialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/editoptiondialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quotepdfgenerator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quotepdfgenerator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/appsettings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/appsettings.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/quotelistdialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quotelistdialog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/emaildialog.h
    ${CMAKE_CURRENT_SOURCE_DIR}/emaildialog.cpp
)

# ── Build the executable ──────────────────────────────────────────────────────
qt_add_executable(QuoteGeneration
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    systemsection.h
    systemsection.cpp
    stylemanager.h
    stylemanager.cpp
    basissection.h
    basissection.cpp
    pricesection.h
    pricesection.cpp
    scopesection.cpp
    scopesection.h
    ${CMAKE_CURRENT_SOURCE_DIR}/exclusionssection.h
       ${CMAKE_CURRENT_SOURCE_DIR}/exclusionssection.cpp
       generalconditionssection.h
       generalconditionssection.cpp
       clarificationssection.h
       clarificationssection.cpp
       signaturesection.h
       signaturesection.cpp
       quotepreviewdialog.h
       quotepreviewdialog.cpp
       quotetypedialog.h
       quotetypedialog.cpp
       editoptiondialog.h
       editoptiondialog.cpp
       dropdownmanagerdialog.h
       dropdownmanagerdialog.cpp
       quotepdfgenerator.h
       quotepdfgenerator.cpp
       appsettings.h
       appsettings.cpp
       quotelistdialog.h
       quotelistdialog.cpp
       emaildialog.h
       emaildialog.cpp


)

# ── Link Qt into the executable ───────────────────────────────────────────────
target_link_libraries(QuoteGeneration PRIVATE
    Qt6::Widgets
    Qt6::Sql
    Qt6::PrintSupport
    Qt6::Network
    Qt6::Gui
)
if(WIN32)
    target_link_libraries(QuoteGeneration PRIVATE Mapi32)
endif()

# ── Suppress console window on Windows ───────────────────────────────────────
if(WIN32)
    set_target_properties(QuoteGeneration PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

# ── Required finalisation step for Qt 6 ──────────────────────────────────────
qt_finalize_executable(QuoteGeneration)