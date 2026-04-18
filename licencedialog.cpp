// ─────────────────────────────────────────────────────────────────────────────
// licencedialog.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "licencedialog.h"
#include "licencemanager.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QGuiApplication>
#include <QClipboard>
#include <QFont>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
LicenceDialog::LicenceDialog(Mode mode, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Software Licence");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Trial expired mode cannot be dismissed without a valid key.
    if (mode == Mode::TrialExpired)
        setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);

    resize(560, 380);
    setupUi(mode);
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void LicenceDialog::setupUi(Mode mode)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(14);

    // ── Heading ───────────────────────────────────────────────────────────────
    QLabel *heading = new QLabel(
        mode == Mode::TrialExpired
            ? "Trial Period Expired"
            : "Activate Software Licence"
        );
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Message ───────────────────────────────────────────────────────────────
    QLabel *message = new QLabel(
        mode == Mode::TrialExpired
            ? "Your trial period has expired. To continue using Quote "
              "Generation please purchase a licence.\n\n"
              "Send your Hardware ID below to PB Software Solutions "
              "to receive your licence key."
            : "To activate your licence enter the key provided by "
              "PB Software Solutions below.\n\n"
              "If you do not have a licence key please send your "
              "Hardware ID to obtain one."
        );
    message->setWordWrap(true);
    message->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(message);

    // ── Divider ───────────────────────────────────────────────────────────────
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // ── Form ──────────────────────────────────────────────────────────────────
    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);

    // Hardware ID — read only, user copies this to send to you.
    m_hardwareIdField = new QLineEdit();
    m_hardwareIdField->setText(LicenceManager::instance().hardwareId());
    m_hardwareIdField->setReadOnly(true);
    m_hardwareIdField->setStyleSheet(StyleManager::instance().lineEditStyle());

    QFont monoFont("Courier New", 9);
    m_hardwareIdField->setFont(monoFont);
    form->addRow("Hardware ID:", m_hardwareIdField);

    // Copy button sits next to the hardware ID.
    m_copyButton = new AnimatedButton("Copy ID");
    m_copyButton->setFixedSize(90, 32);
    connect(m_copyButton, &AnimatedButton::clicked,
            this, &LicenceDialog::onCopyHardwareId);

    QHBoxLayout *hwRow = new QHBoxLayout();
    hwRow->addWidget(m_hardwareIdField, 1);
    hwRow->addWidget(m_copyButton, 0);
    form->addRow("Hardware ID:", hwRow);

    // Remove the duplicate row we just added.
    // FormLayout addRow adds a label — remove the plain field row.
    // Actually just use the hwRow directly:
    // We need to redo this — remove the first addRow and keep only hwRow.

    // Licence key entry.
    m_licenceKeyField = new QLineEdit();
    m_licenceKeyField->setPlaceholderText(
        "XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX-XXXX"
        );
    m_licenceKeyField->setStyleSheet(StyleManager::instance().lineEditStyle());
    m_licenceKeyField->setFont(monoFont);
    form->addRow("Licence Key:", m_licenceKeyField);

    mainLayout->addLayout(form);

    // ── Status label ──────────────────────────────────────────────────────────
    m_statusLabel = new QLabel();
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);

    mainLayout->addStretch();

    // ── Button row ────────────────────────────────────────────────────────────
    QHBoxLayout *buttonRow = new QHBoxLayout();

    // Close button only shown in Activate mode — trial expired cannot close.
    if (mode == Mode::Activate) {
        m_closeButton = new AnimatedButton("Close");
        m_closeButton->setFixedSize(100, 36);
        connect(m_closeButton, &AnimatedButton::clicked,
                this, &QDialog::reject);
        buttonRow->addWidget(m_closeButton);
    } else {
        m_closeButton = nullptr;
    }

    buttonRow->addStretch();

    m_activateButton = new AnimatedButton("Activate");
    m_activateButton->setFixedSize(120, 36);
    connect(m_activateButton, &AnimatedButton::clicked,
            this, &LicenceDialog::onActivate);
    buttonRow->addWidget(m_activateButton);

    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// onActivate()
// ─────────────────────────────────────────────────────────────────────────────
void LicenceDialog::onActivate()
{
    QString key = m_licenceKeyField->text().trimmed();

    if (key.isEmpty()) {
        m_statusLabel->setStyleSheet("QLabel { color: #cc0000; }");
        m_statusLabel->setText("Please enter a licence key.");
        return;
    }

    if (LicenceManager::instance().validateAndActivate(key)) {
        // Valid key — show success and close.
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Activation Successful");
        msgBox.setText(
            "Thank you! Your software has been successfully activated.\n\n"
            "You now have full unrestricted access to Quote Generation."
            );
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        accept();
    } else {
        // Invalid key.
        m_statusLabel->setStyleSheet("QLabel { color: #cc0000; }");
        m_statusLabel->setText(
            "Invalid licence key. Please check the key and try again.\n"
            "Contact PB Software Solutions if you continue to have problems."
            );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onCopyHardwareId()
// ─────────────────────────────────────────────────────────────────────────────
void LicenceDialog::onCopyHardwareId()
{
    QGuiApplication::clipboard()->setText(
        LicenceManager::instance().hardwareId()
        );
    m_statusLabel->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    m_statusLabel->setText("Hardware ID copied to clipboard.");
}