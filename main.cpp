// ─────────────────────────────────────────────────────────────────────────────
// main.cpp  —  Application entry point
//
// Initialises the database, checks if first-run setup is needed,
// shows the config dialog if so, then opens the main window.
// ─────────────────────────────────────────────────────────────────────────────

#include "mainwindow.h"
#include "database.h"
#include "configdialog.h"
#include "stylemanager.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setOrganizationName("PB Software Solutions");
    app.setApplicationName("Quote Generation");
    app.setApplicationVersion("1.0.0");

    // ── Initialise database ───────────────────────────────────────────────────
    if (!Database::initialise()) {
        QMessageBox::critical(
            nullptr,
            "Database Error",
            "The application database could not be opened.\n\n"
            "Path: " + Database::databasePath() + "\n\n"
                                             "Please check that the application folder is not read-only."
            );
        return 1;
    }

    // ── First run check ───────────────────────────────────────────────────────
    // Load the config and check if setup has been completed.
    // If configured is false this is the first time the app has been run.
    AppConfig cfg = Database::loadConfig();
    // Apply the saved light/dark mode preference before any UI is shown.
    StyleManager::instance().setDarkMode(cfg.darkMode);
    StyleManager::instance().applyToApplication(&app);

    if (!cfg.configured) {
        // Show the configuration dialog.
        // exec() blocks until the user clicks Save or Cancel.
        ConfigDialog dlg;
        int result = dlg.exec();

        // If the user clicked Cancel on first run we cannot continue —
        // the app needs the config data to function.
        // We warn them and exit cleanly.
        if (result != QDialog::Accepted) {
            QMessageBox::information(
                nullptr,
                "Setup Required",
                "The application cannot run without completing the initial "
                "setup.\n\nPlease restart and complete the configuration."
                );
            return 0;
        }
    }

    // ── Show main window ──────────────────────────────────────────────────────
    MainWindow w;
    w.show();

    return app.exec();
}