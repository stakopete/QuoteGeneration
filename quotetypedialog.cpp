// ─────────────────────────────────────────────────────────────────────────────
// quotetypedialog.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "quotetypedialog.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QButtonGroup>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
QuoteTypeDialog::QuoteTypeDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("New Quote — Select Type");
    setFixedWidth(450);
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void QuoteTypeDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Heading ───────────────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>New Quote</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Instruction ───────────────────────────────────────────────────────────
    QLabel *instruction = new QLabel(
        "Select the type of quote you are preparing. "
        "This determines which clause lists are available "
        "and how the quote is formatted."
        );
    instruction->setWordWrap(true);
    instruction->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(instruction);

    // ── Quote type group ──────────────────────────────────────────────────────
    QGroupBox *typeGroup = new QGroupBox("Quote Type");
    typeGroup->setStyleSheet(StyleManager::instance().groupBoxStyle());
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);
    typeLayout->setSpacing(12);

    // Button group ensures only one radio is selected at a time.
    QButtonGroup *btnGroup = new QButtonGroup(this);

    m_wetRadio = new QRadioButton("Wet Fire Only");
    m_wetRadio->setStyleSheet(
        QString("QRadioButton { color: %1; font-size: 11pt; }")
            .arg(StyleManager::instance().labelColour())
        );

    QLabel *wetDesc = new QLabel(
        "Fire sprinkler systems, hydrants, hose reels and pump systems."
        );
    wetDesc->setWordWrap(true);
    wetDesc->setStyleSheet(
        QString("QLabel { color: %1; font-size: 9pt; "
                "padding-left: 20px; }")
            .arg(StyleManager::instance().labelColour())
        );

    m_dryRadio = new QRadioButton("Dry Fire Only");
    m_dryRadio->setStyleSheet(
        QString("QRadioButton { color: %1; font-size: 11pt; }")
            .arg(StyleManager::instance().labelColour())
        );

    QLabel *dryDesc = new QLabel(
        "Fire detection, alarm systems, EWIS and electronic surveillance."
        );
    dryDesc->setWordWrap(true);
    dryDesc->setStyleSheet(
        QString("QLabel { color: %1; font-size: 9pt; "
                "padding-left: 20px; }")
            .arg(StyleManager::instance().labelColour())
        );

    m_combinedRadio = new QRadioButton("Combined Wet & Dry Fire");
    m_combinedRadio->setStyleSheet(
        QString("QRadioButton { color: %1; font-size: 11pt; }")
            .arg(StyleManager::instance().labelColour())
        );
    m_combinedRadio->setChecked(true);  // Default selection.

    QLabel *combinedDesc = new QLabel(
        "Both wet and dry fire systems quoted together."
        );
    combinedDesc->setWordWrap(true);
    combinedDesc->setStyleSheet(
        QString("QLabel { color: %1; font-size: 9pt; "
                "padding-left: 20px; }")
            .arg(StyleManager::instance().labelColour())
        );

    m_generalRadio = new QRadioButton("General (Non Fire Industry)");
    m_generalRadio->setStyleSheet(
        QString("QRadioButton { color: %1; font-size: 11pt; }")
            .arg(StyleManager::instance().labelColour())
        );

    QLabel *generalDesc = new QLabel(
        "Construction, building, manufacturing or other industries. "
        "No wet/dry fire split — single clause lists throughout."
        );
    generalDesc->setWordWrap(true);
    generalDesc->setStyleSheet(
        QString("QLabel { color: %1; font-size: 9pt; "
                "padding-left: 20px; }")
            .arg(StyleManager::instance().labelColour())
        );

    btnGroup->addButton(m_wetRadio);
    btnGroup->addButton(m_dryRadio);
    btnGroup->addButton(m_combinedRadio);
    btnGroup->addButton(m_generalRadio);

    typeLayout->addWidget(m_wetRadio);
    typeLayout->addWidget(wetDesc);
    typeLayout->addSpacing(4);
    typeLayout->addWidget(m_dryRadio);
    typeLayout->addWidget(dryDesc);
    typeLayout->addSpacing(4);
    typeLayout->addWidget(m_combinedRadio);
    typeLayout->addWidget(combinedDesc);
    typeLayout->addSpacing(4);
    typeLayout->addWidget(m_generalRadio);
    typeLayout->addWidget(generalDesc);

    mainLayout->addWidget(typeGroup);

    // ── Buttons ───────────────────────────────────────────────────────────────
    QHBoxLayout *buttonRow = new QHBoxLayout();

    m_cancelButton = new AnimatedButton("Cancel");
    m_cancelButton->setFixedSize(110, 40);
    connect(m_cancelButton, &AnimatedButton::clicked,
            this, &QDialog::reject);

    m_proceedButton = new AnimatedButton("Proceed");
    m_proceedButton->setFixedSize(110, 40);
    connect(m_proceedButton, &AnimatedButton::clicked,
            this, &QuoteTypeDialog::onProceed);

    buttonRow->addStretch();
    buttonRow->addWidget(m_cancelButton);
    buttonRow->addSpacing(8);
    buttonRow->addWidget(m_proceedButton);
    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// onProceed()
// ─────────────────────────────────────────────────────────────────────────────
void QuoteTypeDialog::onProceed()
{
    accept();
}

// ─────────────────────────────────────────────────────────────────────────────
// selectedType()
// ─────────────────────────────────────────────────────────────────────────────
QString QuoteTypeDialog::selectedType() const
{
    if (m_wetRadio->isChecked())      return "Wet";
    if (m_dryRadio->isChecked())      return "Dry";
    if (m_generalRadio->isChecked())  return "General";
    return "Combined";
}