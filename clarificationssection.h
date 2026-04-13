#ifndef CLARIFICATIONSSECTION_H
#define CLARIFICATIONSSECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// clarificationssection.h
//
// Clarifications section. Allows the user to include any clarifications
// not covered elsewhere in the quote.
//
// If left empty this section does not appear in the final quote.
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"

class QTextEdit;
class QLineEdit;
class QGroupBox;
class QLabel;
class AnimatedButton;

class ClarificationsSection : public QWidget
{
    Q_OBJECT

public:
    explicit ClarificationsSection(QWidget *parent = nullptr);

    // Returns the full clarifications text.
    QString clarificationsText() const;

    // Returns true if the user has added at least one clarification.
    // If false this section is omitted from the final quote.
    bool isComplete() const;

    // Loads previously saved data.
    void loadData(const QString &text);

signals:
    void dataChanged();

private slots:
    void onAddCustomClause();
    void onRemoveLastClause();
    void onClearAll();
    void onCustomTextChanged(const QString &text);

private:
    void setupUi();
    void applyGroupBoxStyle(QGroupBox *group);
    void appendToClarifications(const QString &text);

    // ── Controls ──────────────────────────────────────────────────────────────
    QTextEdit      *m_clarificationsText;
    QLineEdit      *m_customClause;
    QLabel         *m_charCountLabel;
    AnimatedButton *m_addCustomButton;
    AnimatedButton *m_removeButton;
    AnimatedButton *m_clearButton;
};

#endif // CLARIFICATIONSSECTION_H
