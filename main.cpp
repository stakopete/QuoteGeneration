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
#include "appsettings.h"

#include <QApplication>
#include <QMessageBox>
#include <QLoggingCategory>
#include "licencemanager.h"
#include "licencedialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Suppress Qt internal CSS warnings that are not actionable.
    QLoggingCategory::setFilterRules("qt.widgets.stylesheet.warning=false");

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

    // ── Load INI file settings ────────────────────────────────────────────────
    // Must happen before the config dialog or main window so all paths
    // and preferences are available immediately.
    AppSettings::instance().load();

    AppSettings::instance().load();

    // ── Licence / Trial check ─────────────────────────────────────────────────
    // Check if the trial is still valid or the app is licensed.
    // If neither, show the licence dialog and exit if not activated.
    if (!LicenceManager::instance().checkStartup()) {
        // Trial has expired — show the licence dialog.
        // The dialog cannot be dismissed without a valid licence key.
        LicenceDialog dlg(LicenceDialog::Mode::TrialExpired);
        if (dlg.exec() != QDialog::Accepted) {
            // User did not activate — exit.
            return 0;
        }
    }

    // ── Load AppConfig from database ──────────────────────────────────────────
    // Declared once here and used throughout the rest of main().
    AppConfig cfg = Database::loadConfig();

    // ── Sync INI paths into AppConfig ─────────────────────────────────────────
    // The INI file is the primary store for paths and preferences.
    // If the INI has values that differ from the database we update the database.
    bool needsSync = false;

    if (!AppSettings::instance().logoPath().isEmpty() &&
        cfg.logoPath != AppSettings::instance().logoPath()) {
        cfg.logoPath = AppSettings::instance().logoPath();
        needsSync = true;
    }
    if (!AppSettings::instance().signaturePath().isEmpty() &&
        cfg.signaturePath != AppSettings::instance().signaturePath()) {
        cfg.signaturePath = AppSettings::instance().signaturePath();
        needsSync = true;
    }
    if (!AppSettings::instance().companyName().isEmpty() &&
        cfg.companyName != AppSettings::instance().companyName()) {
        cfg.companyName = AppSettings::instance().companyName();
        needsSync = true;
    }
    if (needsSync)
        Database::saveConfig(cfg);

    // ── Apply light/dark mode ─────────────────────────────────────────────────
    StyleManager::instance().setDarkMode(cfg.darkMode);
    StyleManager::instance().applyToApplication(&app);

    // ── First run check ───────────────────────────────────────────────────────
    if (!cfg.configured) {
        ConfigDialog dlg;
        int result = dlg.exec();

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