// ─────────────────────────────────────────────────────────────────────────────
// main.cpp  —  Application entry point
//
// This is the first code that runs when the application starts.
// It sets up the application identity, initialises the database,
// then creates and shows the main window.
// ─────────────────────────────────────────────────────────────────────────────

#include "mainwindow.h"
#include "database.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    // QApplication manages the application's event loop and resources.
    // It must be the very first Qt object created.
    QApplication app(argc, argv);

    // These strings appear in the About dialog and are used by QSettings
    // if we ever need to store user preferences in the Windows registry.
    app.setOrganizationName("PB Software Solutions");
    app.setApplicationName("Quote Generation");
    app.setApplicationVersion("1.0.0");

    // Initialise the database before showing any UI.
    // If this fails we show an error and exit cleanly.
    // The app cannot function without the database so there is no
    // point showing the main window if it fails.
    if (!Database::initialise()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "The application database could not be opened.\n\n"
            "Path: " + Database::databasePath() + "\n\n"
                                             "Please check that the application folder is not read-only."
            );
        return 1;   // Non-zero return signals failure to the operating system
    }

    // Create and show the main window.
    MainWindow w;
    w.show();

    // Hand control to Qt's event loop. This call does not return until
    // the user closes the application.
    return app.exec();
}