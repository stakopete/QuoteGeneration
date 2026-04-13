// ─────────────────────────────────────────────────────────────────────────────
// clarificationssection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "clarificationssection.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ClarificationsSection::ClarificationsSection(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Clarifications</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Note ─────────────────────────────────────────────────────────────────
    QLabel *note = new QLabel(
        "Include any clarifications not covered elsewhere in the quote. "
        "If nothing is added this section will not appear in the final quote."
        );
    note->setWordWrap(true);
    note->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(note);

    // ── Add clarification group ───────────────────────────────────────────────
    QGroupBox *customGroup = new QGroupBox("Add Clarification");
    applyGroupBoxStyle(customGroup);
    QVBoxLayout *customLayout = new QVBoxLayout(customGroup);

    QLabel *customHint = new QLabel(
        "Type each clarification and click Add. "
        "Each entry is limited to 256 characters."
        );
    customHint->setWordWrap(true);
    customLayout->addWidget(customHint);

    QHBoxLayout *customInputRow = new QHBoxLayout();

    m_customClause = new QLineEdit();
    m_customClause->setPlaceholderText(
        "Type a clarification (max 256 characters)..."
        );
    m_customClause->setMaxLength(256);
    connect(m_customClause, &QLineEdit::textChanged,
            this, &ClarificationsSection::onCustomTextChanged);
    customInputRow->addWidget(m_customClause);

    m_addCustomButton = new AnimatedButton("Add");
    m_addCustomButton->setFixedSize(120, 40);
    connect(m_addCustomButton, &AnimatedButton::clicked,
            this, &ClarificationsSection::onAddCustomClause);
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

    // ── Clarifications text group ─────────────────────────────────────────────
    QGroupBox *textGroup = new QGroupBox(
        "Clarifications (as they will appear in the quote)"
        );
    applyGroupBoxStyle(textGroup);
    QVBoxLayout *textLayout = new QVBoxLayout(textGroup);

    QLabel *textHint = new QLabel(
        "Clarifications you add appear below as a numbered list. "
        "You can also type directly into this box."
        );
    textHint->setWordWrap(true);
    textLayout->addWidget(textHint);

    m_clarificationsText = new QTextEdit();
    m_clarificationsText->setPlaceholderText(
        "Clarifications will appear here..."
        );
    m_clarificationsText->setMinimumHeight(200);
    m_clarificationsText->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_clarificationsText->setSizePolicy(
        QSizePolicy::Expanding, QSizePolicy::Expanding
        );
    connect(m_clarificationsText, &QTextEdit::textChanged,
            this, &ClarificationsSection::dataChanged);
    textLayout->addWidget(m_clarificationsText);

    // Buttons.
    m_removeButton = new AnimatedButton("Remove Last");
    m_removeButton->setFixedSize(120, 40);
    connect(m_removeButton, &AnimatedButton::clicked,
            this, &ClarificationsSection::onRemoveLastClause);

    m_clearButton = new AnimatedButton("Clear All");
    m_clearButton->setFixedSize(120, 40);
    connect(m_clearButton, &AnimatedButton::clicked,
            this, &ClarificationsSection::onClearAll);

    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->setContentsMargins(0, 8, 0, 0);
    buttonRow->addStretch();
    buttonRow->addWidget(m_removeButton);
    buttonRow->addSpacing(8);
    buttonRow->addWidget(m_clearButton);
    textLayout->addLayout(buttonRow);

    mainLayout->addWidget(textGroup);
    mainLayout->addStretch();
}

// ─────────────────────────────────────────────────────────────────────────────
// appendToClarifications()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::appendToClarifications(const QString &text)
{
    QString current = m_clarificationsText->toPlainText().trimmed();

    int itemCount = 0;
    if (!current.isEmpty())
        itemCount = current.split("\n").count();

    QString newItem = QString("%1. %2").arg(itemCount + 1).arg(text);

    if (!current.isEmpty())
        current += "\n";
    current += newItem;

    m_clarificationsText->setPlainText(current);
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddCustomClause()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::onAddCustomClause()
{
    QString text = m_customClause->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Empty");
        msgBox.setText("Please type a clarification before clicking Add.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }
    appendToClarifications(text);
    m_customClause->clear();
}

// ─────────────────────────────────────────────────────────────────────────────
// onRemoveLastClause()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::onRemoveLastClause()
{
    QString current = m_clarificationsText->toPlainText().trimmed();
    if (current.isEmpty())
        return;

    QStringList lines = current.split("\n");
    lines.removeLast();

    QStringList renumbered;
    for (int i = 0; i < lines.count(); ++i) {
        QString line = lines[i];
        int dotPos = line.indexOf(". ");
        if (dotPos > 0)
            line = line.mid(dotPos + 2);
        renumbered.append(QString("%1. %2").arg(i + 1).arg(line));
    }

    m_clarificationsText->setPlainText(renumbered.join("\n"));
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onClearAll()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::onClearAll()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Clear All");
    msgBox.setText("Are you sure you want to clear all clarifications?");
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
        m_clarificationsText->clear();
        emit dataChanged();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onCustomTextChanged()
// ─────────────────────────────────────────────────────────────────────────────
void ClarificationsSection::onCustomTextChanged(const QString &text)
{
    int count = text.length();
    m_charCountLabel->setText(
        QString("%1 / 256 characters").arg(count)
        );

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
QString ClarificationsSection::clarificationsText() const
{
    return m_clarificationsText->toPlainText().trimmed();
}

bool ClarificationsSection::isComplete() const
{
    return !m_clarificationsText->toPlainText().trimmed().isEmpty();
}

void ClarificationsSection::loadData(const QString &text)
{
    m_clarificationsText->setPlainText(text);
}