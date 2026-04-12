// ─────────────────────────────────────────────────────────────────────────────
// database.cpp
//
// Implementation of the Database class. All SQL statements live here.
// ─────────────────────────────────────────────────────────────────────────────

#include "database.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>
#include <QDebug>

// ─────────────────────────────────────────────────────────────────────────────
// databasePath()
//
// We store the database file alongside the executable. This makes the
// application portable — copy the folder and it works on another machine.
// ─────────────────────────────────────────────────────────────────────────────
QString Database::databasePath()
{
    return QCoreApplication::applicationDirPath() + "/QuoteGeneration.sqlite";
}

// ─────────────────────────────────────────────────────────────────────────────
// initialise()
//
// Called once from main.cpp before the main window is shown.
// ─────────────────────────────────────────────────────────────────────────────
bool Database::initialise()
{
    // Add a named SQLite connection. Using a name means we can retrieve
    // this same connection anywhere in the app using database(CONNECTION_NAME).
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", CONNECTION_NAME);
    db.setDatabaseName(databasePath());

    if (!db.open()) {
        qCritical() << "Database::initialise - Failed to open database:"
                    << db.lastError().text();
        return false;
    }

    qDebug() << "Database opened at:" << databasePath();

    // WAL mode makes writes faster and reduces corruption risk on crash.
    QSqlQuery pragma(db);
    pragma.exec("PRAGMA journal_mode=WAL");

    // SQLite does not enforce foreign keys by default. Turn it on.
    pragma.exec("PRAGMA foreign_keys=ON");

    if (!createTables())
        return false;

    // Run any schema migrations needed for existing databases.
    // This adds columns that were added after the initial release.
    if (!migrateSchema())
        return false;

    if (!seedDropdownData())
        return false;

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// createTables()
//
// Creates all tables if they do not already exist.
// IF NOT EXISTS makes this safe to call on every launch.
// ─────────────────────────────────────────────────────────────────────────────
bool Database::createTables()
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);

    // ── Quotes table ──────────────────────────────────────────────────────────
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS Quotes (
            id                  INTEGER PRIMARY KEY AUTOINCREMENT,
            created             TEXT NOT NULL,
            quote_date          TEXT NOT NULL,
            site_name           TEXT NOT NULL DEFAULT '',
            title_text          TEXT NOT NULL DEFAULT 'Response to Request for Quotation',
            system_text         TEXT DEFAULT '',
            basis_text          TEXT DEFAULT '',
            scope_text          TEXT DEFAULT '',
            exclusions          TEXT DEFAULT '',
            general_conditions  TEXT DEFAULT '',
            clarifications      TEXT DEFAULT '',
            status              TEXT DEFAULT 'Draft',
            expiry_date         TEXT DEFAULT '',
            email_to            TEXT DEFAULT '',
            logo_on_all_pages   INTEGER DEFAULT 0,
            suppress_warnings   INTEGER DEFAULT 0,
            last_saved          TEXT DEFAULT ''

        )
    )")) {
        qCritical() << "createTables - Quotes:" << q.lastError().text();
        return false;
    }

    // ── PriceItems table ──────────────────────────────────────────────────────
    // One row per line in the Proposed Price grid.
    // ON DELETE CASCADE means if a Quote is deleted, its price rows go too.
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS PriceItems (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            quote_id    INTEGER NOT NULL REFERENCES Quotes(id) ON DELETE CASCADE,
            sort_order  INTEGER NOT NULL DEFAULT 0,
            description TEXT DEFAULT '',
            amount      REAL DEFAULT 0.0
        )
    )")) {
        qCritical() << "createTables - PriceItems:" << q.lastError().text();
        return false;
    }

    // ── DropdownOptions table ─────────────────────────────────────────────────
    // Stores all selectable clauses for every section.
    // The section column is a string key e.g. "basis_wet" or "scope_dry".
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS DropdownOptions (
            id          INTEGER PRIMARY KEY AUTOINCREMENT,
            section     TEXT NOT NULL,
            option_text TEXT NOT NULL
        )
    )")) {
        qCritical() << "createTables - DropdownOptions:" << q.lastError().text();
        return false;
    }

    // ── AppConfig table ───────────────────────────────────────────────────────
    // Single row (id always = 1). Stores user/company details and settings.
    if (!q.exec(R"(
        CREATE TABLE IF NOT EXISTS AppConfig (
            id              INTEGER PRIMARY KEY DEFAULT 1,
            company_name    TEXT DEFAULT '',
            contact_name    TEXT DEFAULT '',
            phone           TEXT DEFAULT '',
            email           TEXT DEFAULT '',
            logo_path       TEXT DEFAULT '',
            tax_label       TEXT DEFAULT 'GST',
            tax_rate        REAL DEFAULT 10.0,
            configured      INTEGER DEFAULT 0,
            dark_mode       INTEGER DEFAULT 0
        )
    )")) {
        qCritical() << "createTables - AppConfig:" << q.lastError().text();
        return false;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// migrateSchema()
//
// Adds any columns that were introduced after the initial database
// was created. This allows existing databases to be upgraded
// automatically without losing data.
//
// SQLite does not support IF NOT EXISTS for ALTER TABLE so we check
// whether the column exists first before trying to add it.
// ─────────────────────────────────────────────────────────────────────────────
bool Database::migrateSchema()
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);

    // ── Check for dark_mode column in AppConfig ───────────────────────────────
    // We query the table info and look for the column name.
    q.exec("PRAGMA table_info(AppConfig)");
    bool hasDarkMode = false;
    while (q.next()) {
        if (q.value("name").toString() == "dark_mode") {
            hasDarkMode = true;
            break;
        }
    }

    // Add the column if it doesn't exist.
    if (!hasDarkMode) {
        if (!q.exec("ALTER TABLE AppConfig ADD COLUMN dark_mode INTEGER DEFAULT 0")) {
            qWarning() << "migrateSchema - failed to add dark_mode column:"
                       << q.lastError().text();
            return false;
        }
        qDebug() << "migrateSchema - added dark_mode column to AppConfig";
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// seedDropdownData()
//
// Inserts default clauses on first run only.
// Checks if rows exist first — if they do, skips seeding entirely.
// ─────────────────────────────────────────────────────────────────────────────
bool Database::seedDropdownData()
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery check(db);
    check.exec("SELECT COUNT(*) FROM DropdownOptions");
    if (check.next() && check.value(0).toInt() > 0)
        return true;    // Already seeded — nothing to do

    // Helper lambda — inserts one option row.
    // A lambda is a small anonymous function defined inline.
    // The [&] captures the db variable from the surrounding scope.
    auto insert = [&](const QString &section, const QString &text) -> bool {
        QSqlQuery q(db);
        q.prepare("INSERT INTO DropdownOptions (section, option_text) "
                  "VALUES (?, ?)");
        q.addBindValue(section);
        q.addBindValue(text);
        if (!q.exec()) {
            qWarning() << "seedDropdownData insert failed:"
                       << q.lastError().text();
            return false;
        }
        return true;
    };

    // ── Basis of Proposal — Wet Fire ─────────────────────────────────────────
    insert("basis_wet", "AS 2118.1:2017 – Automatic Fire Sprinkler Systems – General Systems");
    insert("basis_wet", "AS 2118.4 – Sprinkler Systems for Residential Occupancies");
    insert("basis_wet", "AS 2118.6 – Combined Sprinkler and Hydrant Systems in High-Rise Buildings");
    insert("basis_wet", "Relevant Standards: NCC2022 V1, AS3000, AS2444, AS1851, AS1670.1 and AS 2118.6-2012");
    insert("basis_wet", "Architectural Drawings: Add Numbers here");
    insert("basis_wet", "We have not had sight of any fire engineering in this early tender process and quotation is subject to review.");

    // ── Basis of Proposal — Dry Fire ─────────────────────────────────────────
    insert("basis_dry", "AS 1670.1:2018 – System Design, Installation & Commissioning");
    insert("basis_dry", "AS 1670.4 – Emergency Warning and Intercommunication Systems (EWIS)");
    insert("basis_dry", "AS/NZS 3000 (Wiring Rules) – Ensures electrical safety and compliance for all cabling and power supply");
    insert("basis_dry", "AS 1851:2012 – Routine Service of Fire Protection Systems");
    insert("basis_dry", "National Construction Code (NCC) Integration");
    insert("basis_dry", "Drawings: add numbers here");

    // ── Scope of Works — Wet Fire ─────────────────────────────────────────────
    insert("scope_wet", "Diesel fire pump.");
    insert("scope_wet", "Boosters assembly including cabinet.");
    insert("scope_wet", "Sprinkler control valve to supply basement levels.");
    insert("scope_wet", "Sprinkler control valve to supply residential areas.");
    insert("scope_wet", "Hydrants to all levels.");
    insert("scope_wet", "Exposed sprinklers and hose reels to basements.");
    insert("scope_wet", "Semi recessed sprinkler throughout.");
    insert("scope_wet", "Concealed space sprinklers protecting mechanical ductwork within the rooms.");
    insert("scope_wet", "Plastic penetrations for balcony heads.");
    insert("scope_wet", "Fire rating of all required penetrations including certification.");
    insert("scope_wet", "Preparation and submission of detailed workshop drawings and as-builts.");
    insert("scope_wet", "Block plans for the pump room and booster assembly.");
    insert("scope_wet", "Operations & maintenance manuals.");
    insert("scope_wet", "Commissioning and testing.");
    insert("scope_wet", "12 months maintenance.");

    // ── Scope of Works — Dry Fire ─────────────────────────────────────────────
    insert("scope_dry", "New Main addressable fire detection control and indicator panel with combined emergency warning system (FDCIE/EWCIE). Ground floor entry area.");
    insert("scope_dry", "Allowance for carpark ventilation fan controls within FDCIE enclosure. Including module interface with MSSB in the basement.");
    insert("scope_dry", "Allowance for interface with sprinkler system, isolation valves and pressure switches. Including connection to each level's sprinkler control valves and flow switches.");
    insert("scope_dry", "Allowance for Addressable smoke detectors in accordance with AS1670.1.");
    insert("scope_dry", "Allowance for evacuation speakers in all corridors, SOU bedrooms and horn speakers in accordance with AS 1670.1.");
    insert("scope_dry", "External Red strobe located at designated building entry point for QFES.");
    insert("scope_dry", "Portable Fire Appliances in accordance with AS2444.");
    insert("scope_dry", "Fire cables, conduit works in slabs, fixings and sundries.");
    insert("scope_dry", "Allowance to fire rate all required fire system penetrations, register and form 12 certification.");
    insert("scope_dry", "Commissioning of fire system and interface performance testing with other associated trades.");
    insert("scope_dry", "Shop and as-installed drawings.");
    insert("scope_dry", "Commissioning documentation (Form 15 & 12) baseline data and installers statements.");
    insert("scope_dry", "QFES inspection.");
    insert("scope_dry", "FDCIE block diagram.");
    insert("scope_dry", "Operations & Maintenance Manuals.");
    insert("scope_dry", "12 months warranty.");
    insert("scope_dry", "Client training (1hr on site allowance).");
    insert("scope_dry", "12 months maintenance in accordance with AS1851 for the DLP.");

    // ── Exclusions — Wet Fire ─────────────────────────────────────────────────
    insert("exclusions_wet", "No allowance has been made for the fire pump room.");
    insert("exclusions_wet", "No allowance has been made for a water storage tank if required.");
    insert("exclusions_wet", "No allowance has been made for the concrete slab for the booster cabinet.");
    insert("exclusions_wet", "No allowance has been made for any underground works including the water meter.");
    insert("exclusions_wet", "No allowance has been made for the water supply to a location in the fire pump room.");
    insert("exclusions_wet", "No allowance has been made for drainage points for the hydrant test drain.");
    insert("exclusions_wet", "A Diesel fire pump is not included as part of this quotation.");
    insert("exclusions_wet", "Booster assembly including cabinet is not included in this quotation.");
    insert("exclusions_wet", "No allowances have been made in this quote for Fire Brigade fees & rentals.");

    // ── Exclusions — Dry Fire ─────────────────────────────────────────────────
    insert("exclusions_dry", "No allowances have been made in this quote for installing electronic security systems doorholding.");
    insert("exclusions_dry", "No allowances have been made in this quote for tactical evacuation plans, portable appliances, sliding fire doors interface.");
    insert("exclusions_dry", "No allowances have been made in this quote for Fire Brigade fees & rentals.");

    // ── General Conditions — Wet Fire ─────────────────────────────────────────
    insert("general_wet", "All work will be carried out during normal working hours 6:30am to 3:30pm Monday to Saturday excluding public and award prescribed holidays.");
    insert("general_wet", "The quotation was made on the assumption that unrestricted access to site was available as per the project program.");
    insert("general_wet", "We will provide supervision of all work.");
    insert("general_wet", "We will provide in-house certification (Form 15 & 12) for the installed fire services as required by Australian Standard & appropriate building codes.");
    insert("general_wet", "We operate using up to date SWMS and Safety plans.");

    // ── General Conditions — Dry Fire ─────────────────────────────────────────
    insert("general_dry", "All work will be carried out during normal working hours 6:30am to 3:30pm Monday to Saturday excluding public and award prescribed holidays.");
    insert("general_dry", "Work outside of these periods will be subject to negotiation and will involve a variation to contract being raised.");
    insert("general_dry", "The quotation was made on the assumption that unrestricted access to site was available as per the project program.");
    insert("general_dry", "We will provide supervision of all work.");
    insert("general_dry", "We will provide in-house certification (Form 15 & 12) for the installed fire services as required by Australian Standard & appropriate building codes.");
    insert("general_dry", "We operate using up to date SWMS and Safety plans.");

    // ── Quotation Titles ──────────────────────────────────────────────────────
    insert("quote_title", "Response to Request for Quotation");
    insert("quote_title", "Response to Request for Proposed Fire Suppression Service");
    insert("quote_title", "Response to Request for Proposed Construction");
    insert("quote_title", "Response to Invitation to Tender");
    insert("quote_title", "Preliminary Budget Estimate");

    qDebug() << "Database seeded with default dropdown options.";
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// loadConfig()
// ─────────────────────────────────────────────────────────────────────────────
AppConfig Database::loadConfig()
{
    AppConfig config;
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);
    q.exec("SELECT id, company_name, contact_name, phone, email, logo_path, "
           "tax_label, tax_rate, configured, dark_mode FROM AppConfig WHERE id = 1");

    if (q.next()) {
        config.companyName  = q.value("company_name").toString();
        config.contactName  = q.value("contact_name").toString();
        config.phone        = q.value("phone").toString();
        config.email        = q.value("email").toString();
        config.logoPath     = q.value("logo_path").toString();
        config.taxLabel     = q.value("tax_label").toString();
        config.taxRate      = q.value("tax_rate").toDouble();
        config.configured   = q.value("configured").toInt() == 1;
        config.darkMode     = q.value("dark_mode").toInt() == 1;
    }
    return config;
}

// ─────────────────────────────────────────────────────────────────────────────
// saveConfig()
//
// INSERT OR REPLACE works whether the row exists or not.
// id = 1 ensures we always have exactly one config row.
// ─────────────────────────────────────────────────────────────────────────────
bool Database::saveConfig(const AppConfig &config)
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);
    q.prepare(R"(
        INSERT OR REPLACE INTO AppConfig
            (id, company_name, contact_name, phone, email, logo_path,
             tax_label, tax_rate, configured, dark_mode)
        VALUES (1, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )");
    q.addBindValue(config.companyName);
    q.addBindValue(config.contactName);
    q.addBindValue(config.phone);
    q.addBindValue(config.email);
    q.addBindValue(config.logoPath);
    q.addBindValue(config.taxLabel);
    q.addBindValue(config.taxRate);
    q.addBindValue(config.configured ? 1 : 0);
    q.addBindValue(config.darkMode ? 1 : 0);

    if (!q.exec()) {
        qWarning() << "saveConfig failed:" << q.lastError().text();
        return false;
    }
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// loadOptions
// ─────────────────────────────────────────────────────────────────────────────
QList<DropdownOption> Database::loadOptions(const QString &section)
{
    QList<DropdownOption> list;
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);
    q.prepare("SELECT id, section, option_text FROM DropdownOptions "
              "WHERE section = ? ORDER BY id");
    q.addBindValue(section);
    q.exec();

    while (q.next()) {
        DropdownOption opt;
        opt.id      = q.value("id").toInt();
        opt.section = q.value("section").toString();
        opt.text    = q.value("option_text").toString();
        list.append(opt);
    }
    return list;
}

// ─────────────────────────────────────────────────────────────────────────────
// addOption()
// ─────────────────────────────────────────────────────────────────────────────
bool Database::addOption(const QString &section, const QString &text)
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);
    q.prepare("INSERT INTO DropdownOptions (section, option_text) "
              "VALUES (?, ?)");
    q.addBindValue(section);
    q.addBindValue(text);
    return q.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
// deleteOption()
// ─────────────────────────────────────────────────────────────────────────────
bool Database::deleteOption(int id)
{
    QSqlDatabase db = QSqlDatabase::database(CONNECTION_NAME);
    QSqlQuery q(db);
    q.prepare("DELETE FROM DropdownOptions WHERE id = ?");
    q.addBindValue(id);
    return q.exec();
}

// ─────────────────────────────────────────────────────────────────────────────
// taxOptions()
//
// Returns the standard tax types for the configuration dialog.
// This is a fixed list — not stored in the database because these are
// well-known global options, not user data.
// ─────────────────────────────────────────────────────────────────────────────
QList<TaxOption> Database::taxOptions()
{
    return {
        { "GST",        "Australia / New Zealand",  10.0 },
        { "VAT",        "UK / Europe",              20.0 },
        { "Sales Tax",  "USA",                       0.0 },
        { "GST",        "Canada",                    5.0 },
        { "No Tax",     "Tax exempt",                0.0 },
        { "Other",      "Enter your own rate",       0.0 }
    };
}