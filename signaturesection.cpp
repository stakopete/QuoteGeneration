// ─────────────────────────────────────────────────────────────────────────────
// signaturesection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "signaturesection.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>
#include <QFileDialog>
#include <QFrame>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
SignatureSection::SignatureSection(QWidget *parent)
    : QWidget(parent)
{
    m_config = Database::loadConfig();
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
// ─────────────────────────────────────────────────────────────────────────────
void SignatureSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void SignatureSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Signature Block</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Contact statement group ───────────────────────────────────────────────
    QGroupBox *statementGroup = new QGroupBox("Contact Statement");
    applyGroupBoxStyle(statementGroup);
    QVBoxLayout *statementLayout = new QVBoxLayout(statementGroup);

    QLabel *statementHint = new QLabel(
        "This statement appears above the signature block in the quote. "
        "It is pre-filled from your configuration but can be edited per quote."
        );
    statementHint->setWordWrap(true);
    statementLayout->addWidget(statementHint);

    // Build the default contact statement from config.
    QString defaultStatement = QString(
                                   "Should you require any further information or clarification on the "
                                   "detail of this quotation then please do not hesitate to contact "
                                   "%1 on %2 or by email at %3."
                                   ).arg(m_config.contactName, m_config.phone, m_config.email);

    m_contactStatement = new QTextEdit();
    m_contactStatement->setPlainText(defaultStatement);
    m_contactStatement->setMaximumHeight(80);
    connect(m_contactStatement, &QTextEdit::textChanged,
            this, &SignatureSection::dataChanged);
    statementLayout->addWidget(m_contactStatement);

    mainLayout->addWidget(statementGroup);

    // ── Signature block group ─────────────────────────────────────────────────
    QGroupBox *sigGroup = new QGroupBox("Signature Block");
    applyGroupBoxStyle(sigGroup);
    QVBoxLayout *sigLayout = new QVBoxLayout(sigGroup);

    // Yours Sincerely label.
    QLabel *yoursLabel = new QLabel("Yours Sincerely,");
    yoursLabel->setStyleSheet(
        QString("QLabel { color: %1; font-style: italic; font-size: 13px; }")
            .arg(StyleManager::instance().labelColour())
        );
    sigLayout->addWidget(yoursLabel);

    sigLayout->addSpacing(8);

    // ── Signature image ───────────────────────────────────────────────────────
    m_signaturePreview = new QLabel();
    m_signaturePreview->setFixedHeight(80);
    m_signaturePreview->setMinimumWidth(300);
    m_signaturePreview->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_signaturePreview->setStyleSheet(
        "QLabel { background-color: white; "
        "border: 1px dashed #999999; padding: 4px; }"
        );
    m_signaturePreview->setText("[ Signature image will appear here ]");
    sigLayout->addWidget(m_signaturePreview);

    // Signature path and browse button.
    m_signaturePathEdit = new QLineEdit();
    m_signaturePathEdit->setReadOnly(true);
    m_signaturePathEdit->setPlaceholderText("No signature image selected");

    m_browseButton = new AnimatedButton("Browse...");
    m_browseButton->setFixedSize(120, 40);
    connect(m_browseButton, &AnimatedButton::clicked,
            this, &SignatureSection::onBrowseSignature);

    m_clearButton = new AnimatedButton("Clear");
    m_clearButton->setFixedSize(120, 40);
    connect(m_clearButton, &AnimatedButton::clicked,
            this, &SignatureSection::onClearSignature);

    QHBoxLayout *browseRow = new QHBoxLayout();
    browseRow->addWidget(m_signaturePathEdit);
    browseRow->addWidget(m_browseButton);
    browseRow->addWidget(m_clearButton);
    sigLayout->addLayout(browseRow);

    sigLayout->addSpacing(8);

    // ── Signatory name ────────────────────────────────────────────────────────
    QFormLayout *nameForm = new QFormLayout();
    nameForm->setLabelAlignment(Qt::AlignRight);

    m_signatoryName = new QLineEdit();
    m_signatoryName->setText(m_config.contactName);
    m_signatoryName->setMaxLength(100);
    connect(m_signatoryName, &QLineEdit::textChanged,
            this, &SignatureSection::dataChanged);
    nameForm->addRow("Name:", m_signatoryName);

    sigLayout->addLayout(nameForm);

    sigLayout->addSpacing(8);

    // ── Separator line ────────────────────────────────────────────────────────
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(
        QString("QFrame { color: %1; }")
            .arg(StyleManager::instance().borderColour())
        );
    sigLayout->addWidget(line);

    // ── Contact details ───────────────────────────────────────────────────────
    // Pre-filled from config — shown below the signature in the quote.
    m_contactDetails = new QLabel(
        QString("<b>%1</b><br>%2<br>%3<br>%4")
            .arg(m_config.companyName,
                 m_config.contactName,
                 m_config.phone,
                 m_config.email)
        );
    m_contactDetails->setStyleSheet(
        QString("QLabel { color: %1; background-color: transparent; }")
            .arg(StyleManager::instance().labelColour())
        );
    sigLayout->addWidget(m_contactDetails);

    mainLayout->addWidget(sigGroup);
    mainLayout->addStretch();

    // Load signature from config if one is saved.
    if (!m_config.signaturePath.isEmpty()) {
        m_signaturePathEdit->setText(m_config.signaturePath);
        loadSignaturePreview(m_config.signaturePath);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// loadSignaturePreview()
// ─────────────────────────────────────────────────────────────────────────────
void SignatureSection::loadSignaturePreview(const QString &path)
{
    QPixmap sig;
    if (sig.load(path)) {
        m_signaturePreview->setPixmap(
            sig.scaledToHeight(70, Qt::SmoothTransformation)
            );
    } else {
        m_signaturePreview->setText("[ Could not load signature image ]");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onBrowseSignature()
// ─────────────────────────────────────────────────────────────────────────────
void SignatureSection::onBrowseSignature()
{
    QString path = QFileDialog::getOpenFileName(
        this,
        "Select Signature Image",
        QString(),
        "PNG Images (*.png)"
        );

    if (!path.isEmpty()) {
        m_signaturePathEdit->setText(path);
        loadSignaturePreview(path);

        // Save the signature path to config so it persists.
        m_config.signaturePath = path;
        Database::saveConfig(m_config);

        emit dataChanged();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onClearSignature()
// ─────────────────────────────────────────────────────────────────────────────
void SignatureSection::onClearSignature()
{
    m_signaturePathEdit->clear();
    m_signaturePreview->setPixmap(QPixmap());
    m_signaturePreview->setText("[ Signature image will appear here ]");

    m_config.signaturePath = "";
    Database::saveConfig(m_config);

    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// Data access
// ─────────────────────────────────────────────────────────────────────────────
QString SignatureSection::contactStatement() const
{
    return m_contactStatement->toPlainText().trimmed();
}

QString SignatureSection::signatoryName() const
{
    return m_signatoryName->text().trimmed();
}

QString SignatureSection::signaturePath() const
{
    return m_signaturePathEdit->text().trimmed();
}

void SignatureSection::loadData(const QString &statement,
                                const QString &name)
{
    if (!statement.isEmpty())
        m_contactStatement->setPlainText(statement);

    if (!name.isEmpty())
        m_signatoryName->setText(name);
}