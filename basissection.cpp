// ─────────────────────────────────────────────────────────────────────────────
// basissection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "basissection.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QSplitter>
#include <QScrollBar>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
BasisSection::BasisSection(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
// ─────────────────────────────────────────────────────────────────────────────
void BasisSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void BasisSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Basis of Proposal</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Note ─────────────────────────────────────────────────────────────────
    QLabel *note = new QLabel(
        "Select applicable standards from the Wet Fire and/or Dry Fire lists. "
        "If nothing is selected this section will not appear in the final quote."
        );
    note->setWordWrap(true);
    note->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(note);

    // ── Clause lists group ────────────────────────────────────────────────────
    // Fix 1: Renamed to "Standard References"
    QGroupBox *listsGroup = new QGroupBox("Standard References");
    applyGroupBoxStyle(listsGroup);
    QGridLayout *listsLayout = new QGridLayout(listsGroup);
    listsLayout->setSpacing(8);

    // ── Wet Fire list ─────────────────────────────────────────────────────────
    m_wetLabel = new QLabel("Wet Fire Standards:");
    listsLayout->addWidget(m_wetLabel, 0, 0);

    m_wetClauseList = new QListWidget();
    m_wetClauseList->setMaximumHeight(150);

    // Fix 2: Style the scroll bars so they are always visible
    // and rendered in the correct colour.
    m_wetClauseList->setStyleSheet(
        StyleManager::instance().listWidgetStyle()
        );

    // Load wet fire clauses from database.
    QList<DropdownOption> wetOptions = Database::loadOptions("basis_wet");
    for (const DropdownOption &opt : wetOptions)
        m_wetClauseList->addItem(opt.text);

    listsLayout->addWidget(m_wetClauseList, 1, 0);

    m_addWetButton = new AnimatedButton("Add Wet Standard");
    m_addWetButton->setFixedSize(138, 50);
    // Reduce font size slightly to fit the longer text on the button.
    m_addWetButton->setStyleSheet(m_addWetButton->styleSheet() +
                                  "QPushButton { font-size: 10px; }");

    QHBoxLayout *wetBtnRow = new QHBoxLayout();
    wetBtnRow->addStretch();
    wetBtnRow->addWidget(m_addWetButton);
    wetBtnRow->addStretch();
    listsLayout->addLayout(wetBtnRow, 2, 0);

    connect(m_addWetButton, &AnimatedButton::clicked,
            this, &BasisSection::onAddWetClause);

    // ── Dry Fire list ─────────────────────────────────────────────────────────
    m_dryLabel = new QLabel("Dry Fire Standards:");
     listsLayout->addWidget(m_dryLabel, 0, 1);

    m_dryClauseList = new QListWidget();
    m_dryClauseList->setMaximumHeight(150);
    m_dryClauseList->setStyleSheet(
        StyleManager::instance().listWidgetStyle()
        );

    // Load dry fire clauses from database.
    QList<DropdownOption> dryOptions = Database::loadOptions("basis_dry");
    for (const DropdownOption &opt : dryOptions)
        m_dryClauseList->addItem(opt.text);

    listsLayout->addWidget(m_dryClauseList, 1, 1);

    m_addDryButton = new AnimatedButton("Add Dry Standard");
    m_addDryButton->setFixedSize(138, 50);
    m_addDryButton->setStyleSheet(m_addDryButton->styleSheet() +
                                  "QPushButton { font-size: 10px; }");

    QHBoxLayout *dryBtnRow = new QHBoxLayout();
    dryBtnRow->addStretch();
    dryBtnRow->addWidget(m_addDryButton);
    dryBtnRow->addStretch();
    listsLayout->addLayout(dryBtnRow, 2, 1);

    connect(m_addDryButton, &AnimatedButton::clicked,
            this, &BasisSection::onAddDryClause);

    mainLayout->addWidget(listsGroup);

    // ── Custom clause group ───────────────────────────────────────────────────
    QGroupBox *customGroup = new QGroupBox("Add Custom Standard or Reference");
    applyGroupBoxStyle(customGroup);
    QHBoxLayout *customLayout = new QHBoxLayout(customGroup);
    customLayout->setSpacing(8);

    m_customClause = new QLineEdit();
    m_customClause->setPlaceholderText("Type a custom standard or reference...");
    m_customClause->setMaxLength(256);

    // Fix 4: Add button same size as others
    m_addCustomButton = new AnimatedButton("Add");
    m_addCustomButton->setFixedSize(138, 50);
    connect(m_addCustomButton, &AnimatedButton::clicked,
            this, &BasisSection::onAddCustomClause);

    customLayout->addWidget(m_customClause);
    customLayout->addWidget(m_addCustomButton);

    mainLayout->addWidget(customGroup);

    // ── Basis text group ──────────────────────────────────────────────────────
    QGroupBox *textGroup = new QGroupBox(
        "Basis of Proposal (as it will appear in the quote)"
        );
    applyGroupBoxStyle(textGroup);
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);

    QLabel *textHint = new QLabel(
        "Standards you add appear below. "
        "You can also type directly into this box."
        );
    textHint->setWordWrap(true);
    textLayout->addWidget(textHint);

    m_basisText = new QTextEdit();
    m_basisText->setPlaceholderText(
        "Selected standards and references will appear here..."
        );
    m_basisText->setMinimumHeight(100);
    m_basisText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Use Expanding horizontally but Preferred vertically so the text
    // edit grows with available space but does not push buttons out.
    m_basisText->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Preferred
        );
    connect(m_basisText, &QTextEdit::textChanged,
            this, &BasisSection::dataChanged);
    textLayout->addWidget(m_basisText);

    mainLayout->addWidget(textGroup, 1);  // stretch=1 lets it grow

    // ── Buttons anchored below the text group ─────────────────────────────────
    m_removeButton = new AnimatedButton("Remove Last");
    m_removeButton->setFixedSize(138, 50);
    connect(m_removeButton, &AnimatedButton::clicked,
            this, &BasisSection::onRemoveLastClause);

    m_clearButton = new AnimatedButton("Clear All");
    m_clearButton->setFixedSize(138, 50);
    connect(m_clearButton, &AnimatedButton::clicked, this, [this]() {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Clear All");
        msgBox.setText("Are you sure you want to clear all basis clauses?");
        msgBox.setIcon(QMessageBox::Question);

        AnimatedButton *yesBtn = new AnimatedButton("Yes", &msgBox);
        yesBtn->setFixedSize(110, 40);
        AnimatedButton *noBtn  = new AnimatedButton("No",  &msgBox);
        noBtn->setFixedSize(110, 40);

        msgBox.addButton(yesBtn,  QMessageBox::YesRole);
        msgBox.addButton(noBtn,   QMessageBox::NoRole);
        msgBox.setDefaultButton(noBtn);
        msgBox.exec();

        if (msgBox.clickedButton() == yesBtn) {
            m_basisText->clear();
            emit dataChanged();
        }
    });

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->setContentsMargins(0, 4, 0, 0);
    buttonRow->addStretch();
    buttonRow->addWidget(m_removeButton);
    buttonRow->addSpacing(8);
    buttonRow->addWidget(m_clearButton);
    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// addClauseFromList()
//
// Shared helper used by both onAddWetClause and onAddDryClause.
// Gets the selected item from the given list and appends it to the
// basis text area.
// ─────────────────────────────────────────────────────────────────────────────
void BasisSection::addClauseFromList(QListWidget *list)
{
    QListWidgetItem *item = list->currentItem();
    if (!item) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select a standard from the list first.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    QString current = m_basisText->toPlainText().trimmed();
    if (!current.isEmpty())
        current += "\n";
    current += item->text();
    m_basisText->setPlainText(current);
    emit dataChanged();
}

void BasisSection::onAddWetClause()
{
    addClauseFromList(m_wetClauseList);
}

void BasisSection::onAddDryClause()
{
    addClauseFromList(m_dryClauseList);
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddCustomClause()
// ─────────────────────────────────────────────────────────────────────────────
void BasisSection::onAddCustomClause()
{
    QString text = m_customClause->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Empty");
        msgBox.setText("Please type a standard or reference before clicking Add.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    QString current = m_basisText->toPlainText().trimmed();
    if (!current.isEmpty())
        current += "\n";
    current += text;
    m_basisText->setPlainText(current);
    m_customClause->clear();
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onRemoveLastClause()
// ─────────────────────────────────────────────────────────────────────────────
void BasisSection::onRemoveLastClause()
{
    QString current = m_basisText->toPlainText().trimmed();
    if (current.isEmpty())
        return;

    QStringList lines = current.split("\n");
    lines.removeLast();
    m_basisText->setPlainText(lines.join("\n"));
    emit dataChanged();
}
//────────────────────────────────────────
// Data access
// ─────────────────────────────────────────────────────────────────────────────
QString BasisSection::basisText() const
{
    return m_basisText->toPlainText().trimmed();
}

bool BasisSection::isComplete() const
{
    return !m_basisText->toPlainText().trimmed().isEmpty();
}

// Strips [WET] [DRY] [GEN] tags from text for display purposes.
// Tags are kept in storage for grouping in combined quotes.
static QString stripTagsForDisplay(const QString &text)
{
    QString result = text;
    result.replace("[WET] ", "");
    result.replace("[DRY] ", "");
    result.replace("[GEN] ", "");
    return result;
}

void BasisSection::loadData(const QString &text)
{
   m_basisText->setPlainText(stripTagsForDisplay(text));
}

void BasisSection::setQuoteType(const QString &type)
{
    bool showWet = (type == "Wet" || type == "Combined");
    bool showDry = (type == "Dry" || type == "Combined");

    m_wetLabel->setVisible(showWet);
    m_wetClauseList->setVisible(showWet);
    m_addWetButton->setVisible(showWet);

    m_dryLabel->setVisible(showDry);
    m_dryClauseList->setVisible(showDry);
    m_addDryButton->setVisible(showDry);
}