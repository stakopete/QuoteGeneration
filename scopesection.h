#ifndef SCOPESECTION_H
#define SCOPESECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// scopesection.h
//
// Scope of Works section. Allows the user to build up a list of work
// items that form part of the quotation.
//
// Separate clause lists are provided for Wet Fire and Dry Fire.
// Custom clauses are limited to 256 characters as per the specification.
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"

class QListWidget;
class QTextEdit;
class QLineEdit;
class QGroupBox;
class QLabel;
class AnimatedButton;

class ScopeSection : public QWidget
{
    Q_OBJECT

public:
    explicit ScopeSection(QWidget *parent = nullptr);

    // Returns the full scope text as it will appear in the quote.
    QString scopeText() const;

    // Scope is always included in the quote.
    bool isComplete() const;

    // Loads previously saved data.
    void loadData(const QString &text);

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
    //void addClauseFromList(QListWidget *list);
    void appendToScope(const QString &text, const QString &systemType = "");

    // ── Controls ──────────────────────────────────────────────────────────────
    QListWidget    *m_wetClauseList;
    QListWidget    *m_dryClauseList;
    QTextEdit      *m_scopeText;
    QLineEdit      *m_customClause;
    QLabel         *m_charCountLabel;
    AnimatedButton *m_addWetButton;
    AnimatedButton *m_addDryButton;
    AnimatedButton *m_addCustomButton;
    AnimatedButton *m_removeButton;
    AnimatedButton *m_clearButton;
};

#endif // SCOPESECTION_H