#ifndef GENERALCONDITIONSSECTION_H
#define GENERALCONDITIONSSECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// generalconditionssection.h
//
// General Conditions section. Allows the user to state any general
// conditions such as site access, hours of work etc.
//
// If left empty this section does not appear in the final quote.
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QGroupBox;
class QLabel;
class AnimatedButton;

class GeneralConditionsSection : public QWidget
{
    Q_OBJECT

public:
    explicit GeneralConditionsSection(QWidget *parent = nullptr);

    // Returns the full general conditions text.
    QString generalText() const;

    // Returns true if the user has added at least one condition.
    // If false this section is omitted from the final quote.
    bool isComplete() const;

    // Loads previously saved data.
    void loadData(const QString &text);

    void setQuoteType(const QString &type);

signals:
    void dataChanged();

private slots:
    void onAddWetClause();
    void onAddDryClause();
    void onAddCustomClause();
    void onRemoveLastClause();
    void onClearAll();
    void onCustomTextChanged(const QString &text);

private:
    void setupUi();
    void applyGroupBoxStyle(QGroupBox *group);
    void appendToGeneral(const QString &text, const QString &systemType);

    // ── Controls ──────────────────────────────────────────────────────────────
    QListWidget    *m_wetClauseList;
    QListWidget    *m_dryClauseList;
    QTextEdit      *m_generalText;
    QLineEdit      *m_customClause;
    QLabel         *m_charCountLabel;
    AnimatedButton *m_addWetButton;
    AnimatedButton *m_addDryButton;
    AnimatedButton *m_addCustomButton;
    AnimatedButton *m_removeButton;
    AnimatedButton *m_clearButton;

    QLabel         *m_wetLabel;
    QLabel         *m_dryLabel;
};

#endif // GENERALCONDITIONSSECTION_H