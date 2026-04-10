#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

// ─────────────────────────────────────────────────────────────────────────────
// configdialog.h
//
// First-run configuration dialog. Collects company and user details,
// logo selection, and tax settings. Writes to the AppConfig table.
//
// This dialog is shown automatically on first launch when configured = 0.
// It can also be opened manually from the application settings menu.
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>
#include "database.h"

// Forward declarations — avoids pulling in heavy headers here.
// Qt will find the full definitions when it needs them in the .cpp file.
class QLineEdit;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QPushButton;

class ConfigDialog : public QDialog
{
    // Q_OBJECT is required for any class that uses signals and slots.
    // It tells Qt's meta-object compiler (moc) to process this class.
    Q_OBJECT

public:
    // Parent is the window that owns this dialog.
    // Passing a parent centres the dialog over that window automatically.
    explicit ConfigDialog(QWidget *parent = nullptr);

    // Returns the config as filled in by the user.
    // Call this after exec() returns QDialog::Accepted.
    AppConfig config() const;

private slots:
    // Called when the user changes the tax type dropdown.
    // Automatically updates the tax rate field with the default for
    // the selected tax type.
    void onTaxTypeChanged(int index);

    // Called when the user clicks the Browse button next to the logo field.
    // Opens a file picker filtered to PNG files.
    void onBrowseLogo();

    // Called when the user clicks Save.
    // Validates all fields and accepts the dialog if everything is filled in.
    void onSave();

private:
    // ── Helper methods ────────────────────────────────────────────────────────
    void setupUi();
    void loadExistingConfig();
    void populateTaxDropdown();
    bool validateInputs();

    // ── UI Controls ───────────────────────────────────────────────────────────
    // QLineEdit is a single-line text input field.
    QLineEdit      *m_companyName;
    QLineEdit      *m_contactName;
    QLineEdit      *m_phone;
    QLineEdit      *m_email;
    QLineEdit      *m_logoPath;

    // QComboBox is a dropdown selector.
    QComboBox      *m_taxType;

    // QDoubleSpinBox is a number field that accepts decimal values.
    // We use it for the tax rate (e.g. 10.0, 20.0).
    QDoubleSpinBox *m_taxRate;

    // Buttons
    QPushButton    *m_browseButton;
    QPushButton    *m_saveButton;
    QPushButton    *m_cancelButton;

    // ── Data ─────────────────────────────────────────────────────────────────
    // Stores the tax options loaded from the database class.
    QList<TaxOption> m_taxOptions;
};

#endif // CONFIGDIALOG_H