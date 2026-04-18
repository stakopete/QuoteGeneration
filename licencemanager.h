#ifndef LICENCEMANAGER_H
#define LICENCEMANAGER_H

// ─────────────────────────────────────────────────────────────────────────────
// licencemanager.h
//
// Handles trial period enforcement and licence key validation.
//
// Trial limits:
//   - 30 days from first run, OR
//   - 10 quote generations
//   whichever comes first.
//
// Licence validation:
//   - Reads motherboard serial via WMI
//   - Combines with Windows product ID
//   - Creates MD5 hardware fingerprint
//   - Validates licence key against fingerprint + secret salt
// ─────────────────────────────────────────────────────────────────────────────

#include <QString>

class LicenceManager
{
public:
    // ── Singleton access ──────────────────────────────────────────────────────
    static LicenceManager &instance();

    // ── Trial status ──────────────────────────────────────────────────────────

    // Returns true if the app is fully licensed.
    bool isLicensed() const;

    // Returns true if the trial is still valid.
    bool isTrialValid() const;

    // Returns the number of days remaining in the trial.
    // Returns 0 if expired.
    int trialDaysRemaining() const;

    // Returns the number of quote generations remaining in the trial.
    int trialQuotesRemaining() const;

    // Call this each time a quote is saved/generated.
    // Increments the quote count and checks trial limits.
    void onQuoteGenerated();

    // Returns a human readable trial status string for display.
    QString trialStatusString() const;

    // ── Hardware fingerprint ──────────────────────────────────────────────────

    // Returns the unique hardware ID for this PC.
    // This is shown to the user so they can send it to you for licensing.
    QString hardwareId() const;

    // ── Licence key ───────────────────────────────────────────────────────────

    // Validates the entered licence key against the hardware ID.
    // Returns true if valid and saves the licence state.
    bool validateAndActivate(const QString &key);

    // ── Startup check ─────────────────────────────────────────────────────────

    // Call on startup. Returns true if the app is allowed to run.
    // Returns false if trial has expired and no valid licence exists.
    bool checkStartup();

private:
    LicenceManager() = default;

    // ── Internal helpers ──────────────────────────────────────────────────────
    QString motherboardSerial() const;
    QString windowsProductId()  const;
    QString generateExpectedKey(const QString &hwId) const;
};

#endif // LICENCEMANAGER_H
