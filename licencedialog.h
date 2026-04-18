#ifndef LICENCEDIALOG_H
#define LICENCEDIALOG_H

// ─────────────────────────────────────────────────────────────────────────────
// licencedialog.h
//
// Shown when the trial has expired or when the user wants to enter
// a licence key. Displays the hardware ID so the user can send it
// to obtain a licence key.
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>

class QLineEdit;
class QLabel;
class AnimatedButton;

class LicenceDialog : public QDialog
{
    Q_OBJECT

public:
    // mode controls whether this is shown as a trial-expired warning
    // or as a voluntary licence entry from the menu.
    enum class Mode {
        TrialExpired,   // Trial has expired — must enter key to continue
        Activate        // User chose to activate from menu
    };

    explicit LicenceDialog(Mode mode, QWidget *parent = nullptr);

private slots:
    void onActivate();
    void onCopyHardwareId();

private:
    void setupUi(Mode mode);

    QLineEdit      *m_hardwareIdField;
    QLineEdit      *m_licenceKeyField;
    QLabel         *m_statusLabel;
    AnimatedButton *m_activateButton;
    AnimatedButton *m_copyButton;
    AnimatedButton *m_closeButton;
};

#endif // LICENCEDIALOG_H
