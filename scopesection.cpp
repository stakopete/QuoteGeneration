// ─────────────────────────────────────────────────────────────────────────────
// scopesection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "scopesection.h"
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

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ScopeSection::ScopeSection(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Scope of Works</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Available clauses group ───────────────────────────────────────────────
    QGroupBox *listsGroup = new QGroupBox("Select Scope of Work Items");
    applyGroupBoxStyle(listsGroup);
    QGridLayout *listsLayout = new QGridLayout(listsGroup);
    listsLayout->setSpacing(8);

    // ── Wet Fire list ─────────────────────────────────────────────────────────
    m_wetLabel = new QLabel("Wet Fire Scope Items:");
    listsLayout->addWidget(m_wetLabel, 0, 0);

    m_wetClauseList = new QListWidget();
    m_wetClauseList->setMaximumHeight(180);
    m_wetClauseList->setStyleSheet(
        StyleManager::instance().listWidgetStyle()
        );

    QList<DropdownOption> wetOptions = Database::loadOptions("scope_wet");
    for (const DropdownOption &opt : wetOptions)
        m_wetClauseList->addItem(opt.text);

    listsLayout->addWidget(m_wetClauseList, 1, 0);

    m_addWetButton = new AnimatedButton("Add Wet Item");
    //m_addWetButton->setFixedSize(138, 50);
    m_addWetButton->setFixedSize(120, 40);
    connect(m_addWetButton, &AnimatedButton::clicked,
            this, &ScopeSection::onAddWetClause);

    QHBoxLayout *wetBtnRow = new QHBoxLayout();
    wetBtnRow->addStretch();
    wetBtnRow->addWidget(m_addWetButton);
    wetBtnRow->addStretch();
    listsLayout->addLayout(wetBtnRow, 2, 0);

    // ── Dry Fire list ─────────────────────────────────────────────────────────
    m_dryLabel = new QLabel("Dry Fire Scope Items:");
    listsLayout->addWidget(m_dryLabel, 0, 1);

    m_dryClauseList = new QListWidget();
    m_dryClauseList->setMaximumHeight(180);
    m_dryClauseList->setStyleSheet(
        StyleManager::instance().listWidgetStyle()
        );

    QList<DropdownOption> dryOptions = Database::loadOptions("scope_dry");
    for (const DropdownOption &opt : dryOptions)
        m_dryClauseList->addItem(opt.text);

    listsLayout->addWidget(m_dryClauseList, 1, 1);

    m_addDryButton = new AnimatedButton("Add Dry Item");
    m_addDryButton->setFixedSize(120, 40);
    connect(m_addDryButton, &AnimatedButton::clicked,
            this, &ScopeSection::onAddDryClause);

    QHBoxLayout *dryBtnRow = new QHBoxLayout();
    dryBtnRow->addStretch();
    dryBtnRow->addWidget(m_addDryButton);
    dryBtnRow->addStretch();
    listsLayout->addLayout(dryBtnRow, 2, 1);

    mainLayout->addWidget(listsGroup);

    // ── Custom clause group ───────────────────────────────────────────────────
    QGroupBox *customGroup = new QGroupBox(
        "Add Custom Scope Item (you will be asked if it is Wet or Dry Fire)"
        );
    applyGroupBoxStyle(customGroup);
    QVBoxLayout *customLayout = new QVBoxLayout(customGroup);

    QLabel *customHint = new QLabel(
        "Custom items are limited to 256 characters including spaces."
        );
    customHint->setWordWrap(true);
    customLayout->addWidget(customHint);

    QHBoxLayout *customInputRow = new QHBoxLayout();

    m_customClause = new QLineEdit();
    m_customClause->setPlaceholderText(
        "Type a custom scope item (max 256 characters)..."
        );
    m_customClause->setMaxLength(256);
    connect(m_customClause, &QLineEdit::textChanged,
            this, &ScopeSection::onCustomTextChanged);
    customInputRow->addWidget(m_customClause);

    m_addCustomButton = new AnimatedButton("Add");
    m_addCustomButton->setFixedSize(120, 40);
    connect(m_addCustomButton, &AnimatedButton::clicked,
            this, &ScopeSection::onAddCustomClause);
    customInputRow->addWidget(m_addCustomButton);

    customLayout->addLayout(customInputRow);

    // Character counter.
    m_charCountLabel = new QLabel("0 / 256 characters");
    m_charCountLabel->setStyleSheet(
        QString("QLabel { color: %1; font-size: 11px; }")
            .arg(StyleManager::instance().labelColour())
        );
    m_charCountLabel->setAlignment(Qt::AlignRight);
    customLayout->addWidget(m_charCountLabel);

    mainLayout->addWidget(customGroup);

    // ── Scope text group ──────────────────────────────────────────────────────
    QGroupBox *textGroup = new QGroupBox(
        "Scope of Works (as it will appear in the quote)"
        );
    applyGroupBoxStyle(textGroup);
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);

    QLabel *textHint = new QLabel(
        "Items you add appear below as a numbered list. "
        "You can also type directly into this box."
        );
    textHint->setWordWrap(true);
    textLayout->addWidget(textHint);

    m_scopeText = new QTextEdit();
    m_scopeText->setPlaceholderText(
        "Scope of works items will appear here..."
        );
    m_scopeText->setMinimumHeight(150);
    m_scopeText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_scopeText->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding
        );
    connect(m_scopeText, &QTextEdit::textChanged,
            this, &ScopeSection::dataChanged);
    textLayout->addWidget(m_scopeText);

    // Buttons.
    m_removeButton = new AnimatedButton("Remove Last");
    m_removeButton->setFixedSize(120, 40);
    connect(m_removeButton, &AnimatedButton::clicked,
            this, &ScopeSection::onRemoveLastClause);

    m_clearButton = new AnimatedButton("Clear All");
    m_clearButton->setFixedSize(120, 40);
    connect(m_clearButton, &AnimatedButton::clicked,
            this, &ScopeSection::onClearAll);

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
// appendToScope()
//
// Appends a new item to the scope text as a numbered list entry.
// We number items automatically so the user doesn't have to.
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::appendToScope(const QString &text,
                                 const QString &systemType)
{
    QString current = m_scopeText->toPlainText().trimmed();

    int itemCount = 0;
    if (!current.isEmpty())
        itemCount = current.split("\n").count();

    // Prefix with system type tag so preview/PDF can group them.
    // [WET] and [DRY] tags are stripped when displaying but used
    // for grouping. Custom items get [GEN] tag.
    QString tag = "";
    if (systemType == "wet")       tag = "[WET] ";
    else if (systemType == "dry")  tag = "[DRY] ";
    else                           tag = "[GEN] ";

    QString newItem = QString("%1. %2%3")
                          .arg(itemCount + 1)
                          .arg(tag)
                          .arg(text);

    if (!current.isEmpty())
        current += "\n";
    current += newItem;

    m_scopeText->setPlainText(current);
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// addClauseFromList()
// ─────────────────────────────────────────────────────────────────────────────
// void ScopeSection::addClauseFromList(QListWidget *list)
// {
//     QListWidgetItem *item = list->currentItem();
//     if (!item) {
//         QMessageBox msgBox(this);
//         msgBox.setWindowTitle("No Selection");
//         msgBox.setText("Please select an item from the list first.");
//         msgBox.setIcon(QMessageBox::Information);
//         AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
//         okBtn->setFixedSize(110, 40);
//         msgBox.addButton(okBtn, QMessageBox::AcceptRole);
//         msgBox.exec();
//         return;
//     }
//     appendToScope(item->text());
// }

void ScopeSection::onAddWetClause()
{
    QListWidgetItem *item = m_wetClauseList->currentItem();
    if (!item) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select an item from the list first.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }
    appendToScope(item->text(), "wet");
}

void ScopeSection::onAddDryClause()
{
    QListWidgetItem *item = m_dryClauseList->currentItem();
    if (!item) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select an item from the list first.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }
    appendToScope(item->text(), "dry");
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddCustomClause()
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::onAddCustomClause()
{
    QString text = m_customClause->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Empty");
        msgBox.setText("Please type a scope item before clicking Add.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    // Ask the user whether this is a Wet or Dry scope item.
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("System Type");
    msgBox.setText(
        "Is this a Wet Fire or Dry Fire scope item?\n\n"
        "This determines how it is grouped in the final quote."
        );
    msgBox.setIcon(QMessageBox::Question);

    AnimatedButton *wetBtn = new AnimatedButton("Wet Fire", &msgBox);
    wetBtn->setFixedSize(110, 40);
    AnimatedButton *dryBtn = new AnimatedButton("Dry Fire", &msgBox);
    dryBtn->setFixedSize(110, 40);
    AnimatedButton *cancelBtn = new AnimatedButton("Cancel", &msgBox);
    cancelBtn->setFixedSize(110, 40);

    msgBox.addButton(wetBtn,    QMessageBox::YesRole);
    msgBox.addButton(dryBtn,    QMessageBox::NoRole);
    msgBox.addButton(cancelBtn, QMessageBox::RejectRole);

    msgBox.exec();

    QAbstractButton *clicked = msgBox.clickedButton();
    if (clicked == cancelBtn)
        return;

    QString systemType = (clicked == wetBtn) ? "wet" : "dry";
    appendToScope(text, systemType);
    m_customClause->clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// onRemoveLastClause()
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::onRemoveLastClause()
{
    QString current = m_scopeText->toPlainText().trimmed();
    if (current.isEmpty())
        return;

    QStringList lines = current.split("\n");
    lines.removeLast();

    // Renumber remaining items.
    QStringList renumbered;
    for (int i = 0; i < lines.count(); ++i) {
        QString line = lines[i];
        // Strip existing number prefix and re-apply.
        int dotPos = line.indexOf(". ");
        if (dotPos > 0)
            line = line.mid(dotPos + 2);
        renumbered.append(QString("%1. %2").arg(i + 1).arg(line));
    }

    m_scopeText->setPlainText(renumbered.join("\n"));
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onClearAll()
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::onClearAll()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Clear All");
    msgBox.setText("Are you sure you want to clear all scope items?");
    msgBox.setIcon(QMessageBox::Question);

    AnimatedButton *yesBtn = new AnimatedButton("Yes", &msgBox);
    yesBtn->setFixedSize(110, 40);
    AnimatedButton *noBtn = new AnimatedButton("No", &msgBox);
    noBtn->setFixedSize(110, 40);

    msgBox.addButton(yesBtn, QMessageBox::YesRole);
    msgBox.addButton(noBtn,  QMessageBox::NoRole);
    msgBox.setDefaultButton(noBtn);
    msgBox.exec();

    if (msgBox.clickedButton() == yesBtn) {
        m_scopeText->clear();
        emit dataChanged();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onCustomTextChanged()
//
// Updates the character counter as the user types.
// ─────────────────────────────────────────────────────────────────────────────
void ScopeSection::onCustomTextChanged(const QString &text)
{
    int count = text.length();
    m_charCountLabel->setText(
        QString("%1 / 256 characters").arg(count)
        );

    // Turn the counter red when approaching the limit.
    if (count >= 240) {
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
QString ScopeSection::scopeText() const
{
    return m_scopeText->toPlainText().trimmed();
}

bool ScopeSection::isComplete() const
{
    return !m_scopeText->toPlainText().trimmed().isEmpty();
}

void ScopeSection::loadData(const QString &text)
{
    m_scopeText->setPlainText(text);
}

void ScopeSection::setQuoteType(const QString &type)
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