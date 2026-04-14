#ifndef BASISSECTION_H
#define BASISSECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// basissection.h
//
// Basis of Proposal section. Allows the user to select the standards
// and references that apply to the quote.
//
// If left empty this section does not appear in the final quote.
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QGroupBox;
class AnimatedButton;
class QLabel;

class BasisSection : public QWidget
{
    Q_OBJECT

public:
    explicit BasisSection(QWidget *parent = nullptr);

    // Returns the full basis text as it will appear in the quote.
    QString basisText() const;

    // Returns true if the user has added at least one clause.
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

private:
    void setupUi();
    void applyGroupBoxStyle(QGroupBox *group);
    void addClauseFromList(QListWidget *list);

    // ── Controls ──────────────────────────────────────────────────────────────
    QListWidget    *m_wetClauseList;
    QListWidget    *m_dryClauseList;
    QTextEdit      *m_basisText;
    QLineEdit      *m_customClause;
    QLabel         *m_charCountLabel;
    QLabel         *m_wetLabel;
    QLabel         *m_dryLabel;
    AnimatedButton *m_addWetButton;
    AnimatedButton *m_addDryButton;
    AnimatedButton *m_addCustomButton;
    AnimatedButton *m_removeButton;
    AnimatedButton *m_clearButton;
};

#endif // BASISSECTION_H