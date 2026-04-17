// ─────────────────────────────────────────────────────────────────────────────
// appsettings.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "appsettings.h"

#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QDate>

// ─────────────────────────────────────────────────────────────────────────────
// instance()
// ─────────────────────────────────────────────────────────────────────────────
AppSettings &AppSettings::instance()
{
    static AppSettings inst;
    return inst;
}

// ─────────────────────────────────────────────────────────────────────────────
// iniFilePath()
//
// The INI file lives alongside the executable.
// This makes the application fully portable.
// ─────────────────────────────────────────────────────────────────────────────
QString AppSettings::iniFilePath()
{
    return QCoreApplication::applicationDirPath()
    + "/QuoteGeneration.ini";
}

// ─────────────────────────────────────────────────────────────────────────────
// load()
//
// Reads all settings from the INI file.
// If the file doesn't exist QSettings creates it automatically when save()
// is first called. Default values are applied for any missing keys.
// ─────────────────────────────────────────────────────────────────────────────
void AppSettings::load()
{
    QSettings s(iniFilePath(), QSettings::IniFormat);

    // ── Paths ─────────────────────────────────────────────────────────────────
    // Default PDF output path — the folder specified in the project spec.
    m_pdfOutputPath  = s.value("Paths/pdf_output",
                              "D:/Qt_Projects/QuoteGeneration/QGenPdfFolder")
                          .toString();
    m_sumatraPdfPath = s.value("Paths/sumatra_pdf", "").toString();
    m_logoPath       = s.value("Paths/logo_path",   "").toString();
    m_signaturePath  = s.value("Paths/signature_path", "").toString();

    // ── Preferences ───────────────────────────────────────────────────────────
    m_darkMode       = s.value("Preferences/dark_mode",       false).toBool();
    m_taxLabel       = s.value("Preferences/tax_label",       "GST").toString();
    m_taxRate        = s.value("Preferences/tax_rate",        10.0).toDouble();
    m_logoOnAllPages = s.value("Preferences/logo_on_all_pages", false).toBool();

    // ── Company ───────────────────────────────────────────────────────────────
    m_companyName    = s.value("Company/name",    "").toString();
    m_contactName    = s.value("Company/contact", "").toString();
    m_phone          = s.value("Company/phone",   "").toString();
    m_email          = s.value("Company/email",   "").toString();

    // ── Trial ─────────────────────────────────────────────────────────────────
    m_firstRunDate   = s.value("Trial/first_run_date", "").toString();
    m_quoteCount     = s.value("Trial/quote_count",    0).toInt();
    m_licensed       = s.value("Trial/licensed",       false).toBool();
    m_licenceKey     = s.value("Trial/licence_key",    "").toString();

    // If this is the very first run record today's date.
    if (m_firstRunDate.isEmpty()) {
        m_firstRunDate = QDate::currentDate().toString("dd/MM/yyyy");
        save();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// save()
//
// Writes all current settings to the INI file.
// ─────────────────────────────────────────────────────────────────────────────
void AppSettings::save()
{
    QSettings s(iniFilePath(), QSettings::IniFormat);

    // ── Paths ─────────────────────────────────────────────────────────────────
    s.setValue("Paths/pdf_output",      m_pdfOutputPath);
    s.setValue("Paths/sumatra_pdf",     m_sumatraPdfPath);
    s.setValue("Paths/logo_path",       m_logoPath);
    s.setValue("Paths/signature_path",  m_signaturePath);

    // ── Preferences ───────────────────────────────────────────────────────────
    s.setValue("Preferences/dark_mode",         m_darkMode);
    s.setValue("Preferences/tax_label",         m_taxLabel);
    s.setValue("Preferences/tax_rate",          m_taxRate);
    s.setValue("Preferences/logo_on_all_pages", m_logoOnAllPages);

    // ── Company ───────────────────────────────────────────────────────────────
    s.setValue("Company/name",    m_companyName);
    s.setValue("Company/contact", m_contactName);
    s.setValue("Company/phone",   m_phone);
    s.setValue("Company/email",   m_email);

    // ── Trial ─────────────────────────────────────────────────────────────────
    s.setValue("Trial/first_run_date", m_firstRunDate);
    s.setValue("Trial/quote_count",    m_quoteCount);
    s.setValue("Trial/licensed",       m_licensed);
    s.setValue("Trial/licence_key",    m_licenceKey);

    // Flush immediately — don't wait for destructor.
    s.sync();
}

// ─────────────────────────────────────────────────────────────────────────────
// incrementQuoteCount()
//
// Called each time a new quote is generated/saved.
// Saves immediately so the count is preserved even if the app crashes.
// ─────────────────────────────────────────────────────────────────────────────
void AppSettings::incrementQuoteCount()
{
    m_quoteCount++;
    save();
}