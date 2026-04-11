// ─────────────────────────────────────────────────────────────────────────────
// systemsection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "systemsection.h"
#include "animatedbutton.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
SystemSection::SystemSection(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
//
// Applies consistent styling to all group boxes in this section.
// Defined once here and called for each group box.
// ─────────────────────────────────────────────────────────────────────────────
void SystemSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            color: #2c3e50;
            border: 1px solid #999999;
            border-radius: 4px;
            margin-top: 16px;
            padding-top: 12px;
            background-color: #d4d4d4;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 4px;
            color: #2c3e50;
            background-color: transparent;
        }
        QLabel {
            color: #2c3e50;
            background-color: transparent;
        }
        QListWidget {
            color: #2c3e50;
            background-color: white;
            border: 1px solid #999999;
            border-radius: 3px;
        }
        QListWidget::item:selected {
            background-color: #3d5166;
            color: white;
        }
        QListWidget::item:hover {
            background-color: #dde1e7;
        }
        QTextEdit {
            color: #2c3e50;
            background-color: white;
            border: 1px solid #999999;
            border-radius: 3px;
            padding: 4px;
        }
        QLineEdit {
            color: #2c3e50;
            background-color: white;
            border: 1px solid #999999;
            border-radius: 3px;
            padding: 4px;
        }
    )");
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void SystemSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>System Offered</h3>");
    heading->setStyleSheet(R"(
        QLabel {
            background-color: #2c3e50;
            color: white;
            padding: 8px 12px;
            border-radius: 4px;
        }
    )");
    mainLayout->addWidget(heading);

    // ── Available clauses group ───────────────────────────────────────────────
    QGroupBox *clauseGroup = new QGroupBox("Available Tasks");
    applyGroupBoxStyle(clauseGroup);
    QVBoxLayout *clauseLayout = new QVBoxLayout(clauseGroup);

    QLabel *clauseHint = new QLabel(
        "Select a task and click Add Selection, or type your own below."
        );
    clauseHint->setWordWrap(true);
    clauseLayout->addWidget(clauseHint);

    // List of available system clauses.
    // We seed these directly here since the database doesn't have a
    // dedicated system_offered section — the user builds this freely.
    m_clauseList = new QListWidget();
    m_clauseList->addItem("Fire Sprinkler System (Wet Fire) — AS 2118.1");
    m_clauseList->addItem("Fire Alarm & Detection System (Dry Fire) — AS 1670.1");
    m_clauseList->addItem("Combined Wet & Dry Fire System");
    m_clauseList->addItem("Hydrant & Hose Reel System");
    m_clauseList->addItem("Emergency Warning & Intercommunication System (EWIS)");
    m_clauseList->addItem("Portable Fire Appliances — AS 2444");
    m_clauseList->addItem("Special Hazard Suppression System");
    m_clauseList->addItem("Fire Pump System");
    m_clauseList->setMaximumHeight(160);
    clauseLayout->addWidget(m_clauseList);

    // Add selected clause button.
    m_addButton = new AnimatedButton("Add Selection");
    m_addButton->setFixedWidth(120);
    m_addButton->setFixedHeight(40);
    connect(m_addButton, &AnimatedButton::clicked,
            this, &SystemSection::onAddClause);

    QHBoxLayout *addRow = new QHBoxLayout();
    addRow->addWidget(m_addButton);
    addRow->addStretch();
    clauseLayout->addLayout(addRow);

    // ── Custom clause row ─────────────────────────────────────────────────────
    QLabel *customLabel = new QLabel("Custom Task:");
    m_customClause = new QLineEdit();
    m_customClause->setPlaceholderText("Type a custom task...");
    m_customClause->setMaxLength(256);

    m_addCustomButton = new AnimatedButton("Add");
    m_addCustomButton->setFixedWidth(80);
    connect(m_addCustomButton, &AnimatedButton::clicked,
            this, &SystemSection::onAddCustomClause);

    QHBoxLayout *customRow = new QHBoxLayout();
    customRow->addWidget(customLabel);
    customRow->addWidget(m_customClause);
    customRow->addWidget(m_addCustomButton);
    clauseLayout->addLayout(customRow);

    mainLayout->addWidget(clauseGroup);

    // ── System description group ──────────────────────────────────────────────
    QGroupBox *textGroup = new QGroupBox("Systems Offered (As they will appear in the quote)");
    applyGroupBoxStyle(textGroup);
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);

    QLabel *textHint = new QLabel(
        "Tasks you add appear below. You can add tasks by typing directly into this box."
        );
    textHint->setWordWrap(true);
    textLayout->addWidget(textHint);

    m_systemText = new QTextEdit();
    m_systemText->setPlaceholderText("System description will appear here...");
    //m_systemText->setMinimumHeight(120);
    m_systemText->setMinimumHeight(120);
    m_systemText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(m_systemText, &QTextEdit::textChanged,
            this, &SystemSection::dataChanged);
    textLayout->addWidget(m_systemText);

    // Remove last clause button.
    // Remove last button sits below the text edit with a fixed margin.
    // We do NOT use addStretch above it as that causes overlap when
    // the window is resized. Instead we use a fixed spacing.
    m_removeButton = new AnimatedButton("Remove Last");
    m_removeButton->setFixedWidth(120);
    m_removeButton->setFixedHeight(40);
    connect(m_removeButton, &AnimatedButton::clicked,
            this, &SystemSection::onRemoveClause);

    QHBoxLayout *removeRow = new QHBoxLayout();
    removeRow->setContentsMargins(0, 8, 0, 0);  // 8px top margin only
    removeRow->addStretch();
    removeRow->addWidget(m_removeButton);
    textLayout->addLayout(removeRow);

    mainLayout->addWidget(textGroup);
    mainLayout->addStretch();
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddClause()
//
// Adds the selected clause from the list to the system description text.
// ─────────────────────────────────────────────────────────────────────────────
void SystemSection::onAddClause()
{
    QListWidgetItem *item = m_clauseList->currentItem();
    if (!item) {
        QMessageBox::information(this, "No Selection",
                                 "Please select a clause from the list first.");
        return;
    }

    QString current = m_systemText->toPlainText().trimmed();
    if (!current.isEmpty())
        current += "\n";
    current += item->text();
    m_systemText->setPlainText(current);
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddCustomClause()
// ─────────────────────────────────────────────────────────────────────────────
void SystemSection::onAddCustomClause()
{
    QString text = m_customClause->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::information(this, "Empty Clause",
                                 "Please type a clause before clicking Add.");
        return;
    }

    QString current = m_systemText->toPlainText().trimmed();
    if (!current.isEmpty())
        current += "\n";
    current += text;
    m_systemText->setPlainText(current);
    m_customClause->clear();
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onRemoveClause()
//
// Removes the last line from the system description text.
// ─────────────────────────────────────────────────────────────────────────────
void SystemSection::onRemoveClause()
{
    QString current = m_systemText->toPlainText().trimmed();
    if (current.isEmpty())
        return;

    // Split into lines, remove the last one, rejoin.
    QStringList lines = current.split("\n");
    lines.removeLast();
    m_systemText->setPlainText(lines.join("\n"));
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// Data access
// ─────────────────────────────────────────────────────────────────────────────
QString SystemSection::systemText() const
{
    return m_systemText->toPlainText().trimmed();
}

bool SystemSection::isComplete() const
{
    return !m_systemText->toPlainText().trimmed().isEmpty();
}

void SystemSection::loadData(const QString &text)
{
    m_systemText->setPlainText(text);
}