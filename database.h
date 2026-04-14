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
    bool    darkMode        = false;
    QString signaturePath;      // Path to signature PNG file
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

// Represents a complete quote with all its sections.
struct QuoteData {
    int     id              = 0;        // 0 = not yet saved
    QString created;
    QString quoteDate;
    QString siteName;
    QString titleText;
    QString systemText;
    QString basisText;
    QString scopeText;
    QString exclusions;
    QString generalConditions;
    QString clarifications;
    QString contactStatement;
    QString signatoryName;
    QString status          = "Draft";
    QString expiryDate;
    QString emailTo;
    bool    logoOnAllPages  = false;
    bool    suppressWarnings = false;
    QString lastSaved;
    QString quoteType = "Combined";  // Wet, Dry, Combined, General
};

// Represents one row in the price grid.
struct PriceRow {
    QString description;
    double  amount = 0.0;
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

    // ── Quote operations ─────────────────────────────────────────────────────

    // Saves a quote to the database. If quote.id == 0 a new quote is
    // created and the id is set on the returned struct.
    // Returns the saved quote with updated id and lastSaved timestamp.
    static QuoteData    saveQuote(const QuoteData &quote);

    // Loads a quote by id. Returns an empty QuoteData if not found.
    static QuoteData    loadQuote(int id);

    // Returns a list of all quotes (id, siteName, status, lastSaved).
    // Used for the open quote dialog.
    static QList<QuoteData> listQuotes();

    // Deletes a quote and all its price items.
    static bool         deleteQuote(int id);

    // Saves the price items for a quote.
    // Replaces all existing price items for that quote.
    static bool         savePriceItems(int quoteId,
                               const QList<PriceRow> &rows);

    // Loads the price items for a quote.
    static QList<PriceRow> loadPriceItems(int quoteId);

    // Returns the id of the most recently saved quote.
    // Returns 0 if no quotes exist.
    static int          lastQuoteId();

private:
    // ── Private helpers called only from initialise() ─────────────────────────
    static bool createTables();
    static bool seedDropdownData();
    static bool migrateSchema();

    // The Qt connection name — identifies our single database connection.
    static constexpr const char* CONNECTION_NAME = "QuoteGenDB";
};

#endif // DATABASE_H