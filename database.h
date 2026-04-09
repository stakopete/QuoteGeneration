#ifndef DATABASE_H
#define DATABASE_H

// ─────────────────────────────────────────────────────────────────────────────
// database.h
//
// This class owns the single SQLite connection for the entire application.
// Every read and write to persistent storage goes through here.
//
// Design principle: one class, one responsibility. The rest of the application
// never touches QSqlDatabase directly — it calls methods on this class.
// ─────────────────────────────────────────────────────────────────────────────

#include <QString>
#include <QList>

// ── Simple data-carrying structs ──────────────────────────────────────────────
// These hold data retrieved from the database and pass it around the app.
// They are plain structs — no logic, just data.

struct AppConfig {
    int     id              = 1;
    QString companyName;
    QString contactName;
    QString phone;
    QString email;
    QString logoPath;
    QString taxLabel        = "GST";
    double  taxRate         = 10.0;
    bool    configured      = false;
};

struct TaxOption {
    QString label;          // e.g. "GST", "VAT", "Sales Tax"
    QString hint;           // e.g. "Australia", "UK/Europe" shown in UI only
    double  defaultRate;    // suggested rate when this type is selected
};

struct DropdownOption {
    int     id          = 0;
    QString section;    // e.g. "basis_wet", "scope_dry", "exclusions_wet"
    QString text;
};

// ── Database class ────────────────────────────────────────────────────────────

class Database
{
public:
    // Call once at application startup. Opens (or creates) the SQLite file,
    // creates tables if they don't exist, and seeds default dropdown data
    // if this is the very first run.
    // Returns true on success, false if the database could not be opened.
    static bool initialise();

    // ── AppConfig operations ─────────────────────────────────────────────────

    // Read the single configuration row. Returns a default AppConfig if
    // the row does not exist yet (first run before setup completes).
    static AppConfig    loadConfig();

    // Write (insert or update) the configuration row.
    static bool         saveConfig(const AppConfig &config);

    // ── Dropdown operations ──────────────────────────────────────────────────

    // Return all dropdown options for a given section name.
    static QList<DropdownOption> loadOptions(const QString &section);

    // Add a new user-defined option to a section.
    static bool addOption(const QString &section, const QString &text);

    // Remove an option by its database id.
    static bool deleteOption(int id);

    // ── Tax options ──────────────────────────────────────────────────────────

    // Returns the built-in list of tax type options for the config dialog.
    static QList<TaxOption> taxOptions();

    // ── Path helper ──────────────────────────────────────────────────────────

    // Returns the full path to the SQLite database file.
    static QString databasePath();

private:
    // ── Private helpers called only from initialise() ─────────────────────────
    static bool createTables();
    static bool seedDropdownData();

    // The Qt connection name — identifies our single database connection.
    static constexpr const char* CONNECTION_NAME = "QuoteGenDB";
};

#endif // DATABASE_H