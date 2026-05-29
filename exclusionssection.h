#ifndef EXCLUSIONSSECTION_H
#define EXCLUSIONSSECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// exclusionssection.h
//
// Exclusions section. Lists what is NOT included in the quotation.
// Custom clauses limited to 128 characters as per specification.
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

class ExclusionsSection : public QWidget
{
    Q_OBJECT

public:
    explicit ExclusionsSection(QWidget *parent = nullptr);

    // Returns the full exclusions text.
    QString exclusionsText() const;

    // Returns true if the user has added at least one exclusion.
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
    void appendToExclusions(const QString &text,
                            const QString &systemType);
    QString m_exclusionsData;  // Internal store with tags preserved.
    // ── Controls ──────────────────────────────────────────────────────────────
    QListWidget    *m_wetClauseList;
    QListWidget    *m_dryClauseList;
    QTextEdit      *m_exclusionsText;
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

#endif // EXCLUSIONSSECTION_H
