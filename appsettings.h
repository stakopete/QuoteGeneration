#ifndef APPSETTINGS_H
#define APPSETTINGS_H

// ─────────────────────────────────────────────────────────────────────────────
// appsettings.h
//
// Centralised INI file management using QSettings.
//
// This class is a singleton — one instance manages the INI file throughout
// the application lifetime. All reads and writes go through here.
//
// The INI file is stored alongside the executable so the application
// remains portable — copy the folder and settings travel with it.
//
// Usage:
//   AppSettings::instance().pdfOutputPath()
//   AppSettings::instance().setPdfOutputPath("/some/path")
//   AppSettings::instance().save()
// ─────────────────────────────────────────────────────────────────────────────

#include <QString>

// Forward declaration — avoids pulling in QSettings header everywhere.
class QSettings;

class AppSettings
{
public:
    // ── Singleton access ──────────────────────────────────────────────────────
    static AppSettings &instance();

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    // Loads settings from the INI file. Call once at startup before any
    // other method. Creates the INI file with defaults if it doesn't exist.
    void load();

    // Writes all current settings back to the INI file.
    void save();

    // Returns the full path to the INI file.
    static QString iniFilePath();

    // ── Paths ─────────────────────────────────────────────────────────────────
    QString pdfOutputPath()    const { return m_pdfOutputPath;    }
    QString sumatraPdfPath()   const { return m_sumatraPdfPath;   }
    QString logoPath()         const { return m_logoPath;         }
    QString signaturePath()    const { return m_signaturePath;    }

    void setPdfOutputPath(const QString &v)  { m_pdfOutputPath   = v; }
    void setSumatraPdfPath(const QString &v) { m_sumatraPdfPath  = v; }
    void setLogoPath(const QString &v)       { m_logoPath        = v; }
    void setSignaturePath(const QString &v)  { m_signaturePath   = v; }

    // ── Preferences ───────────────────────────────────────────────────────────
    bool    darkMode()         const { return m_darkMode;         }
    QString taxLabel()         const { return m_taxLabel;         }
    double  taxRate()          const { return m_taxRate;          }
    bool    logoOnAllPages()   const { return m_logoOnAllPages;   }

    void setDarkMode(bool v)              { m_darkMode       = v; }
    void setTaxLabel(const QString &v)    { m_taxLabel       = v; }
    void setTaxRate(double v)             { m_taxRate        = v; }
    void setLogoOnAllPages(bool v)        { m_logoOnAllPages = v; }

    // ── Company ───────────────────────────────────────────────────────────────
    QString companyName()      const { return m_companyName;      }
    QString contactName()      const { return m_contactName;      }
    QString phone()            const { return m_phone;            }
    QString email()            const { return m_email;            }

    void setCompanyName(const QString &v)  { m_companyName  = v; }
    void setContactName(const QString &v)  { m_contactName  = v; }
    void setPhone(const QString &v)        { m_phone        = v; }
    void setEmail(const QString &v)        { m_email        = v; }

    // ── Trial ─────────────────────────────────────────────────────────────────
    QString firstRunDate()     const { return m_firstRunDate;     }
    int     quoteCount()       const { return m_quoteCount;       }
    bool    isLicensed()       const { return m_licensed;         }
    QString licenceKey()       const { return m_licenceKey;       }

    void setFirstRunDate(const QString &v) { m_firstRunDate = v; }
    void setQuoteCount(int v)              { m_quoteCount   = v; }
    void setLicensed(bool v)              { m_licensed     = v; }
    void setLicenceKey(const QString &v)   { m_licenceKey   = v; }

    // Increments the quote count and saves immediately.
    void incrementQuoteCount();

private:
    // Private constructor — use instance() instead.
    AppSettings() = default;

    // ── Member variables ──────────────────────────────────────────────────────
    // Paths
    QString m_pdfOutputPath;
    QString m_sumatraPdfPath;
    QString m_logoPath;
    QString m_signaturePath;

    // Preferences
    bool    m_darkMode       = false;
    QString m_taxLabel       = "GST";
    double  m_taxRate        = 10.0;
    bool    m_logoOnAllPages = false;

    // Company
    QString m_companyName;
    QString m_contactName;
    QString m_phone;
    QString m_email;

    // Trial
    QString m_firstRunDate;
    int     m_quoteCount  = 0;
    bool    m_licensed    = false;
    QString m_licenceKey;
};

#endif // APPSETTINGS_H