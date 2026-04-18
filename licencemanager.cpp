// ─────────────────────────────────────────────────────────────────────────────
// licencemanager.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "licencemanager.h"
#include "appsettings.h"

#include <QDate>
#include <QCryptographicHash>
#include <QSettings>
#include <QSysInfo>
#include <QProcess>



// ─────────────────────────────────────────────────────────────────────────────
// SECRET SALT — change this to your own private string before distribution.
// This string is never shown to the user and is compiled into the binary.
// It makes licence keys unique to your application.
// Keep this secret — do not share it or commit it to a public repository.
// ─────────────────────────────────────────────────────────────────────────────
static const char *SECRET_SALT = "PBSoftwareSolutions_QG_2026_XK9";

// Trial limits.
static const int TRIAL_DAYS   = 30;
static const int TRIAL_QUOTES = 10;

// ─────────────────────────────────────────────────────────────────────────────
// instance()
// ─────────────────────────────────────────────────────────────────────────────
LicenceManager &LicenceManager::instance()
{
    static LicenceManager inst;
    return inst;
}

// ─────────────────────────────────────────────────────────────────────────────
// isLicensed()
// ─────────────────────────────────────────────────────────────────────────────
bool LicenceManager::isLicensed() const
{
    return AppSettings::instance().isLicensed();
}

// ─────────────────────────────────────────────────────────────────────────────
// isTrialValid()
// ─────────────────────────────────────────────────────────────────────────────
bool LicenceManager::isTrialValid() const
{
    // If licensed, always valid.
    if (isLicensed())
        return true;

    // Check days remaining.
    if (trialDaysRemaining() <= 0)
        return false;

    // Check quote count.
    if (trialQuotesRemaining() <= 0)
        return false;

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// trialDaysRemaining()
// ─────────────────────────────────────────────────────────────────────────────
int LicenceManager::trialDaysRemaining() const
{
    QString firstRunStr = AppSettings::instance().firstRunDate();
    if (firstRunStr.isEmpty())
        return TRIAL_DAYS;

    QDate firstRun = QDate::fromString(firstRunStr, "dd/MM/yyyy");
    if (!firstRun.isValid())
        return TRIAL_DAYS;

    int daysSinceFirstRun = firstRun.daysTo(QDate::currentDate());
    int remaining = TRIAL_DAYS - daysSinceFirstRun;
    return qMax(0, remaining);
}

// ─────────────────────────────────────────────────────────────────────────────
// trialQuotesRemaining()
// ─────────────────────────────────────────────────────────────────────────────
int LicenceManager::trialQuotesRemaining() const
{
    int used = AppSettings::instance().quoteCount();
    return qMax(0, TRIAL_QUOTES - used);
}

// ─────────────────────────────────────────────────────────────────────────────
// onQuoteGenerated()
//
// Call each time a quote is saved. Increments the trial quote counter.
// ─────────────────────────────────────────────────────────────────────────────
void LicenceManager::onQuoteGenerated()
{
    if (isLicensed())
        return;   // No limit tracking needed once licensed.

    AppSettings::instance().incrementQuoteCount();
}

// ─────────────────────────────────────────────────────────────────────────────
// trialStatusString()
// ─────────────────────────────────────────────────────────────────────────────
QString LicenceManager::trialStatusString() const
{
    if (isLicensed())
        return "Licensed";

    int days   = trialDaysRemaining();
    int quotes = trialQuotesRemaining();

    if (days <= 0 || quotes <= 0)
        return "Trial Expired";

    return QString("Trial — %1 day%2 / %3 quote%4 remaining")
        .arg(days)
        .arg(days == 1 ? "" : "s")
        .arg(quotes)
        .arg(quotes == 1 ? "" : "s");
}

// ─────────────────────────────────────────────────────────────────────────────
// motherboardSerial()
//
// Reads the motherboard serial number using Windows WMI.
// Returns empty string if WMI is unavailable.
// ─────────────────────────────────────────────────────────────────────────────
QString LicenceManager::motherboardSerial() const
{
#ifdef Q_OS_WIN
    // Use QProcess to run a WMI query via wmic command line tool.
    // This avoids the MSVC-only COM/WMI headers that don't work with MinGW.
    QProcess process;
    process.start("wmic", QStringList()
                              << "baseboard"
                              << "get"
                              << "SerialNumber"
                              << "/value");
    process.waitForFinished(5000);

    QString output = QString::fromLocal8Bit(process.readAllStandardOutput());

    // Output format is: SerialNumber=XXXXXXXX
    // We extract the value after the equals sign.
    for (const QString &line : output.split('\n')) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("SerialNumber=")) {
            QString serial = trimmed.mid(13).trimmed();
            if (!serial.isEmpty())
                return serial;
        }
    }
    return QString("UNKNOWN");
#else
    return QString();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// windowsProductId()
//
// Reads the Windows Product ID from the registry.
// This gives us a second hardware-tied identifier.
// ─────────────────────────────────────────────────────────────────────────────
QString LicenceManager::windowsProductId() const
{
#ifdef Q_OS_WIN
    QSettings reg(
        "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        QSettings::NativeFormat
        );
    return reg.value("ProductId").toString();
#else
    return QSysInfo::machineUniqueId();
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// hardwareId()
//
// Creates a unique hardware fingerprint for this PC.
// Combines motherboard serial + Windows product ID then MD5 hashes the result.
// The hash is formatted as groups of 4 characters for readability.
// ─────────────────────────────────────────────────────────────────────────────
QString LicenceManager::hardwareId() const
{
    QString raw = motherboardSerial() + "|" + windowsProductId();

    // MD5 hash the combined string.
    QByteArray hash = QCryptographicHash::hash(
                          raw.toUtf8(), QCryptographicHash::Md5
                          ).toHex().toUpper();

    // Format as XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX for readability.
    QString formatted;
    for (int i = 0; i < hash.length(); i++) {
        if (i > 0 && i % 4 == 0)
            formatted += '-';
        formatted += hash[i];
    }
    return formatted;
}

// ─────────────────────────────────────────────────────────────────────────────
// generateExpectedKey()
//
// Generates the expected licence key for a given hardware ID.
// This is the same algorithm used in the key generator tool.
// The secret salt ensures keys are unique to this application.
// ─────────────────────────────────────────────────────────────────────────────
QString LicenceManager::generateExpectedKey(const QString &hwId) const
{
    // Remove dashes from hardware ID for hashing.
    QString cleanId = hwId;
    cleanId.remove('-');

    // Combine hardware ID with secret salt.
    QString combined = cleanId + SECRET_SALT;

    // SHA-256 hash the combination.
    QByteArray hash = QCryptographicHash::hash(
                          combined.toUtf8(), QCryptographicHash::Sha256
                          ).toHex().toUpper();

    // Take the first 32 characters and format as XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX.
    QString key;
    for (int i = 0; i < 32; i++) {
        if (i > 0 && i % 4 == 0)
            key += '-';
        key += hash[i];
    }
    return key;
}

// ─────────────────────────────────────────────────────────────────────────────
// validateAndActivate()
//
// Validates the entered licence key against this PC's hardware ID.
// If valid saves the licence state to the INI file.
// ─────────────────────────────────────────────────────────────────────────────
bool LicenceManager::validateAndActivate(const QString &key)
{
    QString expected = generateExpectedKey(hardwareId());

    // Compare case-insensitively and ignore dashes.
    QString cleanKey      = key.toUpper().remove('-').remove(' ');
    QString cleanExpected = expected.toUpper().remove('-');

    if (cleanKey != cleanExpected)
        return false;

    // Valid — save the licence state.
    AppSettings::instance().setLicensed(true);
    AppSettings::instance().setLicenceKey(key);
    AppSettings::instance().save();
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// checkStartup()
//
// Called on startup. Returns true if the app is allowed to run.
// ─────────────────────────────────────────────────────────────────────────────
bool LicenceManager::checkStartup()
{
    // Already licensed — always allow.
    if (isLicensed())
        return true;

    // Trial still valid — allow.
    if (isTrialValid())
        return true;

    // Trial expired — do not allow.
    return false;
}