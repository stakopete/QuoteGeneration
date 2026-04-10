// ─────────────────────────────────────────────────────────────────────────────
// configdialog.cpp
//
// Implementation of the first-run configuration dialog.
// ─────────────────────────────────────────────────────────────────────────────

#include "configdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
//
// QDialog::setWindowTitle sets the text in the dialog's title bar.
// setupUi() builds all the controls.
// loadExistingConfig() fills in any previously saved values.
// ─────────────────────────────────────────────────────────────────────────────
ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Application Configuration");

    // Fixed size — we don't want this dialog to be resizable.
    setFixedWidth(650);

    setupUi();
    loadExistingConfig();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
//
// Builds the dialog layout programmatically.
//
// Layout classes used:
// QVBoxLayout  = arranges widgets vertically (top to bottom)
// QHBoxLayout  = arranges widgets horizontally (left to right)
// QFormLayout  = two-column layout with labels on left, fields on right
//                Perfect for settings forms.
// ─────────────────────────────────────────────────────────────────────────────
void ConfigDialog::setupUi()
{
    // ── Main layout ───────────────────────────────────────────────────────────
    // Everything in the dialog sits inside this vertical layout.
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(16);
    mainLayout->setContentsMargins(24, 24, 24, 24);

    // ── Title label ───────────────────────────────────────────────────────────
    QLabel *titleLabel = new QLabel(
        "<h2>Application Setup</h2>"
        "<p>Please enter your details. These will appear on every quotation.</p>"
        );
    titleLabel->setWordWrap(true);
    mainLayout->addWidget(titleLabel);

    // ── Form layout ───────────────────────────────────────────────────────────
    // QFormLayout automatically pairs labels with their input fields.
    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);
    form->setLabelAlignment(Qt::AlignRight);

    // Company name field
    m_companyName = new QLineEdit();
    m_companyName->setPlaceholderText("Your company name");
    m_companyName->setMaxLength(100);
    form->addRow("Company Name: *", m_companyName);

    // Contact name field
    m_contactName = new QLineEdit();
    m_contactName->setPlaceholderText("Your full name");
    m_contactName->setMaxLength(100);
    form->addRow("Contact Name: *", m_contactName);

    // Phone field
    m_phone = new QLineEdit();
    m_phone->setPlaceholderText("e.g. +61 7 1234 5678");
    m_phone->setMaxLength(30);
    form->addRow("Phone: *", m_phone);

    // Email field
    m_email = new QLineEdit();
    m_email->setPlaceholderText("your@email.com");
    m_email->setMaxLength(100);
    form->addRow("Email: *", m_email);

    // ── Logo selection row ────────────────────────────────────────────────────
    // This row has a text field AND a Browse button side by side,
    // so we use a nested QHBoxLayout inside the form row.
    m_logoPath = new QLineEdit();
    m_logoPath->setPlaceholderText("Optional — leave blank to use default logo");
    m_logoPath->setReadOnly(true);  // User must use the Browse button

    m_browseButton = new QPushButton("Browse...");
    m_browseButton->setFixedWidth(80);

    // Connect the Browse button click to our slot.
    // Qt signals and slots: when the button emits clicked(),
    // our onBrowseLogo() method is called automatically.
    connect(m_browseButton, &QPushButton::clicked,
            this, &ConfigDialog::onBrowseLogo);

    QHBoxLayout *logoRow = new QHBoxLayout();
    logoRow->addWidget(m_logoPath);
    logoRow->addWidget(m_browseButton);
    form->addRow("Logo File:", logoRow);

    // ── Tax type dropdown ─────────────────────────────────────────────────────
    m_taxType = new QComboBox();
    populateTaxDropdown();

    // When the user changes the selection, auto-fill the rate field.
    connect(m_taxType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ConfigDialog::onTaxTypeChanged);

    form->addRow("Tax Type: *", m_taxType);

    // ── Tax rate field ────────────────────────────────────────────────────────
    // QDoubleSpinBox: min 0, max 100, 2 decimal places, % suffix
    m_taxRate = new QDoubleSpinBox();
    m_taxRate->setMinimum(0.0);
    m_taxRate->setMaximum(100.0);
    m_taxRate->setDecimals(2);
    m_taxRate->setSuffix(" %");
    m_taxRate->setValue(10.0);  // Default to GST
    form->addRow("Tax Rate: *", m_taxRate);

    mainLayout->addLayout(form);

    // ── Required fields note ──────────────────────────────────────────────────
    QLabel *requiredNote = new QLabel("* Required fields");
    requiredNote->setStyleSheet("color: grey; font-size: 11px;");
    mainLayout->addWidget(requiredNote);

    // ── Buttons ───────────────────────────────────────────────────────────────
    // QHBoxLayout with a spacer pushes the buttons to the right.
    QHBoxLayout *buttonRow = new QHBoxLayout();

    m_cancelButton = new QPushButton("Cancel");
    m_cancelButton->setFixedWidth(90);
    connect(m_cancelButton, &QPushButton::clicked,
            this, &QDialog::reject);

    m_saveButton = new QPushButton("Save");
    m_saveButton->setFixedWidth(90);
    m_saveButton->setDefault(true);  // Save is triggered by pressing Enter
    connect(m_saveButton, &QPushButton::clicked,
            this, &ConfigDialog::onSave);

    // addStretch() pushes everything after it to the right.
    buttonRow->addStretch();
    buttonRow->addWidget(m_cancelButton);
    buttonRow->addWidget(m_saveButton);

    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// populateTaxDropdown()
//
// Loads tax options from the Database class and adds them to the combobox.
// The display text shows both label and hint e.g. "GST (Australia / NZ)"
// but only the short label is stored in AppConfig.
// ─────────────────────────────────────────────────────────────────────────────
void ConfigDialog::populateTaxDropdown()
{
    m_taxOptions = Database::taxOptions();
    m_taxType->clear();

    for (const TaxOption &opt : m_taxOptions) {
        // Show "GST (Australia / New Zealand)" in the dropdown
        // but store only "GST" in the database.
        m_taxType->addItem(
            QString("%1 (%2)").arg(opt.label, opt.hint)
            );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// loadExistingConfig()
//
// If the user opens this dialog a second time (from Settings menu),
// we pre-fill all fields with the previously saved values.
// ─────────────────────────────────────────────────────────────────────────────
void ConfigDialog::loadExistingConfig()
{
    AppConfig cfg = Database::loadConfig();

    m_companyName->setText(cfg.companyName);
    m_contactName->setText(cfg.contactName);
    m_phone->setText(cfg.phone);
    m_email->setText(cfg.email);
    m_logoPath->setText(cfg.logoPath);
    m_taxRate->setValue(cfg.taxRate);

    // Find and select the matching tax type in the dropdown.
    for (int i = 0; i < m_taxOptions.size(); ++i) {
        if (m_taxOptions[i].label == cfg.taxLabel) {
            m_taxType->setCurrentIndex(i);
            break;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onTaxTypeChanged()
//
// Called automatically when the user picks a different tax type.
// Updates the rate field with the sensible default for that jurisdiction.
// The user can still override the rate manually after this.
// ─────────────────────────────────────────────────────────────────────────────
void ConfigDialog::onTaxTypeChanged(int index)
{
    if (index >= 0 && index < m_taxOptions.size()) {
        m_taxRate->setValue(m_taxOptions[index].defaultRate);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onBrowseLogo()
//
// Opens a file picker dialog filtered to PNG files only.
// If the user selects a file, its path is shown in the logo field.
// ─────────────────────────────────────────────────────────────────────────────
void ConfigDialog::onBrowseLogo()
{
    // QFileDialog::getOpenFileName shows a standard file picker.
    // The filter "PNG Images (*.png)" restricts what the user can select.
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select Logo Image",
        QString(),              // Start in the last used folder
        "PNG Images (*.png)"
        );

    // If the user cancelled, path will be empty — don't update the field.
    if (!path.isEmpty()) {
        m_logoPath->setText(path);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onSave()
//
// Validates all required fields. If anything is missing, shows a warning
// and does not close the dialog. If all is well, saves to the database
// and accepts (closes) the dialog.
// ─────────────────────────────────────────────────────────────────────────────
void ConfigDialog::onSave()
{
    // Validate first — don't save incomplete data.
    if (!validateInputs())
        return;

    // Build the config struct from the form fields.
    AppConfig cfg;
    cfg.companyName = m_companyName->text().trimmed();
    cfg.contactName = m_contactName->text().trimmed();
    cfg.phone       = m_phone->text().trimmed();
    cfg.email       = m_email->text().trimmed();
    cfg.logoPath    = m_logoPath->text().trimmed();
    cfg.taxRate     = m_taxRate->value();
    cfg.configured  = true;    // Mark setup as complete

    // Get the short label from the selected tax option.
    int idx = m_taxType->currentIndex();
    if (idx >= 0 && idx < m_taxOptions.size())
        cfg.taxLabel = m_taxOptions[idx].label;
    else
        cfg.taxLabel = "GST";

    // Save to database.
    if (!Database::saveConfig(cfg)) {
        QMessageBox::warning(
            this,
            "Save Failed",
            "The configuration could not be saved.\n"
            "Please check the application has write access to its folder."
            );
        return;
    }

    // accept() closes the dialog and makes exec() return QDialog::Accepted.
    accept();
}

// ─────────────────────────────────────────────────────────────────────────────
// validateInputs()
//
// Checks that all required fields have been filled in.
// Highlights the first empty field and shows a message to the user.
// Returns true if everything is valid, false if anything is missing.
// ─────────────────────────────────────────────────────────────────────────────
bool ConfigDialog::validateInputs()
{
    // List of required fields and their display names.
    // We check them in order and stop at the first empty one.
    struct Field {
        QLineEdit  *edit;
        QString     name;
    };

    QList<Field> required = {
        { m_companyName, "Company Name" },
        { m_contactName, "Contact Name" },
        { m_phone,       "Phone"        },
        { m_email,       "Email"        }
    };

    for (const Field &f : required) {
        if (f.edit->text().trimmed().isEmpty()) {
            QMessageBox::warning(
                this,
                "Required Field",
                f.name + " cannot be left blank."
                );
            f.edit->setFocus();     // Move cursor to the empty field
            return false;
        }
    }

    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
// config()
//
// Returns the saved configuration. Call this after the dialog closes
// with QDialog::Accepted to retrieve what the user entered.
// ─────────────────────────────────────────────────────────────────────────────
AppConfig ConfigDialog::config() const
{
    return Database::loadConfig();
}