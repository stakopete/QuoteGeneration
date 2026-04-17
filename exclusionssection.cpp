// ─────────────────────────────────────────────────────────────────────────────
// exclusionssection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "exclusionssection.h"
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
#include <QScrollBar>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ExclusionsSection::ExclusionsSection(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Exclusions</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Note ─────────────────────────────────────────────────────────────────
    QLabel *note = new QLabel(
        "List what is NOT included in this quotation. "
        "If nothing is added this section will not appear in the final quote."
        );
    note->setWordWrap(true);
    note->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(note);

    // ── Clause lists group ────────────────────────────────────────────────────
    QGroupBox *listsGroup = new QGroupBox("Select Exclusion Items");
    applyGroupBoxStyle(listsGroup);
    QGridLayout *listsLayout = new QGridLayout(listsGroup);
    listsLayout->setSpacing(8);

    // ── Wet Fire exclusions ───────────────────────────────────────────────────
    m_wetLabel = new QLabel("Wet Fire Exclusions:");
    listsLayout->addWidget(m_wetLabel, 0, 0);

    m_wetClauseList = new QListWidget();
    m_wetClauseList->setMaximumHeight(160);
    m_wetClauseList->setStyleSheet(
        StyleManager::instance().listWidgetStyle()
        );

    QList<DropdownOption> wetOptions = Database::loadOptions("exclusions_wet");
    for (const DropdownOption &opt : wetOptions)
        m_wetClauseList->addItem(opt.text);

    listsLayout->addWidget(m_wetClauseList, 1, 0);

    m_addWetButton = new AnimatedButton("Add Wet Exclusion");
    m_addWetButton->setFixedSize(120, 40);
    connect(m_addWetButton, &AnimatedButton::clicked,
            this, &ExclusionsSection::onAddWetClause);

    QHBoxLayout *wetBtnRow = new QHBoxLayout();
    wetBtnRow->addStretch();
    wetBtnRow->addWidget(m_addWetButton);
    wetBtnRow->addStretch();
    listsLayout->addLayout(wetBtnRow, 2, 0);

    // ── Dry Fire exclusions ───────────────────────────────────────────────────
    m_dryLabel = new QLabel("Dry Fire Exclusions:");
    listsLayout->addWidget(m_dryLabel, 0, 1);

    m_dryClauseList = new QListWidget();
    m_dryClauseList->setMaximumHeight(160);
    m_dryClauseList->setStyleSheet(
        StyleManager::instance().listWidgetStyle()
        );

    QList<DropdownOption> dryOptions = Database::loadOptions("exclusions_dry");
    for (const DropdownOption &opt : dryOptions)
        m_dryClauseList->addItem(opt.text);

    listsLayout->addWidget(m_dryClauseList, 1, 1);

    m_addDryButton = new AnimatedButton("Add Dry Exclusion");
    m_addDryButton->setFixedSize(120, 40);
    connect(m_addDryButton, &AnimatedButton::clicked,
            this, &ExclusionsSection::onAddDryClause);

    QHBoxLayout *dryBtnRow = new QHBoxLayout();
    dryBtnRow->addStretch();
    dryBtnRow->addWidget(m_addDryButton);
    dryBtnRow->addStretch();
    listsLayout->addLayout(dryBtnRow, 2, 1);

    mainLayout->addWidget(listsGroup);

    // ── Custom clause group ───────────────────────────────────────────────────
    // Note: 128 character limit for exclusions (spec requirement).
    QGroupBox *customGroup = new QGroupBox(
        "Add Custom Exclusion (you will be asked if it is Wet or Dry Fire)"
        );
    applyGroupBoxStyle(customGroup);
    QVBoxLayout *customLayout = new QVBoxLayout(customGroup);

    QLabel *customHint = new QLabel(
        "Custom exclusions are limited to 512 characters including spaces."
        );
    customHint->setWordWrap(true);
    customLayout->addWidget(customHint);

    QHBoxLayout *customInputRow = new QHBoxLayout();

    m_customClause = new QLineEdit();
    m_customClause->setPlaceholderText(
        "Type a custom exclusion (max 512 characters)..."
        );
    m_customClause->setMaxLength(512);
    connect(m_customClause, &QLineEdit::textChanged,
            this, &ExclusionsSection::onCustomTextChanged);
    customInputRow->addWidget(m_customClause);

    m_addCustomButton = new AnimatedButton("Add");
    m_addCustomButton->setFixedSize(120, 40);
    connect(m_addCustomButton, &AnimatedButton::clicked,
            this, &ExclusionsSection::onAddCustomClause);
    customInputRow->addWidget(m_addCustomButton);

    customLayout->addLayout(customInputRow);

    // Character counter.
    m_charCountLabel = new QLabel("0 / 512 characters");
    m_charCountLabel->setStyleSheet(
        QString("QLabel { color: %1; font-size: 11px; }")
            .arg(StyleManager::instance().labelColour())
        );
    m_charCountLabel->setAlignment(Qt::AlignRight);
    customLayout->addWidget(m_charCountLabel);

    mainLayout->addWidget(customGroup);

    // ── Exclusions text group ─────────────────────────────────────────────────
    QGroupBox *textGroup = new QGroupBox(
        "Exclusions (as they will appear in the quote)"
        );
    applyGroupBoxStyle(textGroup);
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);

    QLabel *textHint = new QLabel(
        "Items you add appear below as a numbered list. "
        "You can also type directly into this box."
        );
    textHint->setWordWrap(true);
    textLayout->addWidget(textHint);

    m_exclusionsText = new QTextEdit();
    m_exclusionsText->setPlaceholderText(
        "Exclusion items will appear here..."
        );
    m_exclusionsText->setMinimumHeight(120);
    m_exclusionsText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_exclusionsText->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding
        );
    connect(m_exclusionsText, &QTextEdit::textChanged,
            this, &ExclusionsSection::dataChanged);
    textLayout->addWidget(m_exclusionsText);

    // Buttons.
    m_removeButton = new AnimatedButton("Remove Last");
    m_removeButton->setFixedSize(120, 40);
    connect(m_removeButton, &AnimatedButton::clicked,
            this, &ExclusionsSection::onRemoveLastClause);

    m_clearButton = new AnimatedButton("Clear All");
    m_clearButton->setFixedSize(120, 40);
    connect(m_clearButton, &AnimatedButton::clicked,
            this, &ExclusionsSection::onClearAll);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->setContentsMargins(0, 8, 0, 0);
    buttonRow->addStretch();
    buttonRow->addWidget(m_removeButton);
    buttonRow->addSpacing(8);
    buttonRow->addWidget(m_clearButton);
    textLayout->addLayout(buttonRow);

    mainLayout->addWidget(textGroup);
}

// ─────────────────────────────────────────────────────────────────────────────
// appendToExclusions()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::appendToExclusions(const QString &text,
                                           const QString &systemType)
{
    QString current = m_exclusionsText->toPlainText().trimmed();

    int itemCount = 0;
    itemCount = current.split("\n", Qt::SkipEmptyParts).count();

    QString tag = "";
    if (systemType == "wet")      tag = "[WET] ";
    else if (systemType == "dry") tag = "[DRY] ";

    // Strip any newlines from the clause text — each clause must be
    // on a single line so the tag parser can identify it correctly.
    // Long clauses will word-wrap visually in the text box.
    QString cleanText = text;
    cleanText.replace("\n", " ").replace("\r", " ");
    // Collapse any double spaces created by the replacement.
    while (cleanText.contains("  "))
        cleanText.replace("  ", " ");
    cleanText = cleanText.trimmed();

    QString newItem = QString("%1. %2%3")
                          .arg(itemCount + 1)
                          .arg(tag)
                          .arg(cleanText);

    if (!current.isEmpty())
        current += "\n";
    current += newItem;

    m_exclusionsText->setPlainText(current);

    // Scroll to bottom so the newly added item is fully visible.
    m_exclusionsText->verticalScrollBar()->setValue(
        m_exclusionsText->verticalScrollBar()->maximum()
        );

    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddWetClause()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::onAddWetClause()
{
    QListWidgetItem *item = m_wetClauseList->currentItem();
    if (!item) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select an exclusion from the list first.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }
    appendToExclusions(item->text(), "wet");
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddDryClause()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::onAddDryClause()
{
    QListWidgetItem *item = m_dryClauseList->currentItem();
    if (!item) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select an exclusion from the list first.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }
    appendToExclusions(item->text(), "dry");
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddCustomClause()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::onAddCustomClause()
{
    QString text = m_customClause->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Empty");
        msgBox.setText("Please type an exclusion before clicking Add.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    // Ask Wet or Dry.
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("System Type");
    msgBox.setText(
        "Is this a Wet Fire or Dry Fire exclusion?\n\n"
        "This determines how it is grouped in the final quote."
        );
    msgBox.setIcon(QMessageBox::Question);

    AnimatedButton *wetBtn    = new AnimatedButton("Wet Fire", &msgBox);
    wetBtn->setFixedSize(110, 40);
    AnimatedButton *dryBtn    = new AnimatedButton("Dry Fire", &msgBox);
    dryBtn->setFixedSize(110, 40);
    AnimatedButton *cancelBtn = new AnimatedButton("Cancel",   &msgBox);
    cancelBtn->setFixedSize(110, 40);

    msgBox.addButton(wetBtn,    QMessageBox::YesRole);
    msgBox.addButton(dryBtn,    QMessageBox::NoRole);
    msgBox.addButton(cancelBtn, QMessageBox::RejectRole);
    msgBox.exec();

    QAbstractButton *clicked = msgBox.clickedButton();
    if (clicked == cancelBtn)
        return;

    QString systemType = (clicked == wetBtn) ? "wet" : "dry";
    appendToExclusions(text, systemType);
    m_customClause->clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// onRemoveLastClause()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::onRemoveLastClause()
{
    QString current = m_exclusionsText->toPlainText().trimmed();
    if (current.isEmpty())
        return;

    QStringList lines = current.split("\n");
    lines.removeLast();

    // Renumber remaining items.
    QStringList renumbered;
    for (int i = 0; i < lines.count(); ++i) {
        QString line = lines[i];
        int dotPos = line.indexOf(". ");
        if (dotPos > 0)
            line = line.mid(dotPos + 2);
        renumbered.append(QString("%1. %2").arg(i + 1).arg(line));
    }

    m_exclusionsText->setPlainText(renumbered.join("\n"));
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onClearAll()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::onClearAll()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Clear All");
    msgBox.setText("Are you sure you want to clear all exclusions?");
    msgBox.setIcon(QMessageBox::Question);

    AnimatedButton *yesBtn = new AnimatedButton("Yes", &msgBox);
    yesBtn->setFixedSize(110, 40);
    AnimatedButton *noBtn  = new AnimatedButton("No",  &msgBox);
    noBtn->setFixedSize(110, 40);

    msgBox.addButton(yesBtn, QMessageBox::YesRole);
    msgBox.addButton(noBtn,  QMessageBox::NoRole);
    msgBox.setDefaultButton(noBtn);
    msgBox.exec();

    if (msgBox.clickedButton() == yesBtn) {
        m_exclusionsText->clear();
        emit dataChanged();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onCustomTextChanged()
// ─────────────────────────────────────────────────────────────────────────────
void ExclusionsSection::onCustomTextChanged(const QString &text)
{
    int count = text.length();
    m_charCountLabel->setText(
        QString("%1 / 512 characters").arg(count)
        );

    // Turn red when approaching the 128 character limit.
    if (count >= 492) {
        m_charCountLabel->setStyleSheet(
            "QLabel { color: #cc0000; font-size: 11px; }"
            );
    } else {
        m_charCountLabel->setStyleSheet(
            QString("QLabel { color: %1; font-size: 11px; }")
                .arg(StyleManager::instance().labelColour())
            );
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Data access
// ─────────────────────────────────────────────────────────────────────────────
QString ExclusionsSection::exclusionsText() const
{
    return m_exclusionsText->toPlainText().trimmed();
}

bool ExclusionsSection::isComplete() const
{
    return !m_exclusionsText->toPlainText().trimmed().isEmpty();
}

void ExclusionsSection::loadData(const QString &text)
{
    m_exclusionsText->setPlainText(text);
}

void ExclusionsSection::setQuoteType(const QString &type)
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