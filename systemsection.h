#ifndef SYSTEMSECTION_H
#define SYSTEMSECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// systemsection.h
//
// The System Offered section. Broadly describes what system is being
// quoted — Wet Fire, Dry Fire, or both.
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"

class QTextEdit;
class QListWidget;
class AnimatedButton;
class QLineEdit;
class QGroupBox;

class SystemSection : public QWidget
{
    Q_OBJECT

public:
    explicit SystemSection(QWidget *parent = nullptr);

    // Returns the full system description text.
    QString systemText() const;

    // Returns true if the user has entered something.
    bool isComplete() const;

    // Loads previously saved data.
    void loadData(const QString &text);

signals:
    void dataChanged();

private slots:
    void onAddClause();
    void onAddCustomClause();
    void onRemoveClause();

private:
    void setupUi();
    void applyGroupBoxStyle(QGroupBox *group);

    // ── Controls ──────────────────────────────────────────────────────────────
    QListWidget    *m_clauseList;    // Dropdown list of available clauses
    QTextEdit      *m_systemText;   // The built-up system description
    QLineEdit      *m_customClause; // For typing a custom clause
    AnimatedButton *m_addButton;
    AnimatedButton *m_addCustomButton;
    AnimatedButton *m_removeButton;
};

#endif // SYSTEMSECTION_H