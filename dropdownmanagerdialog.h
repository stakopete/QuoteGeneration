#ifndef DROPDOWNMANAGERDIALOG_H
#define DROPDOWNMANAGERDIALOG_H

// ---------------------------------------------------------------------------
// DropdownManagerDialog
// ---------------------------------------------------------------------------
// The main Dropdown Manager settings dialog, accessed via Tools → Settings.
//
// Layout (two-panel design):
//
//   ┌─────────────────────────────────────────────────────────────┐
//   │  Dropdown Option Manager                                    │
//   ├───────────────────────┬─────────────────────────────────────┤
//   │  SECTIONS             │  OPTIONS FOR: <section name>        │
//   │  ─────────────────    │  ──────────────────────────────     │
//   │  Basis Wet        ◄── │  Option text one                    │
//   │  Basis Dry            │  Option text two                    │
//   │  Scope Wet            │  Option text three                  │
//   │  Scope Dry            │                                     │
//   │  Exclusions Wet       │                                     │
//   │  Exclusions Dry       │                                     │
//   │  General Wet          │                     [Add]           │
//   │  General Dry          │                     [Edit]          │
//   │  Quote Title          │                     [Delete]        │
//   └───────────────────────┴─────────────────────────────────────┘
//                                               [Close]
//
// How it works:
//   1. The left list shows human-readable section names.
//   2. Clicking a section loads its options from SQLite into the right list.
//   3. Add opens EditOptionDialog and inserts the result via Database::addOption().
//   4. Edit opens EditOptionDialog pre-filled and updates via delete+re-insert
//      (SQLite has no UPDATE with our current API, so we delete then add).
//   5. Delete asks for confirmation then calls Database::deleteOption(id).
//
// The DropdownOption struct (from database.h) holds:  int id, QString text.
// ---------------------------------------------------------------------------

#include <QDialog>
#include <QList>

// Forward declarations
class QListWidget;
class QListWidgetItem;
class QLabel;
class QSplitter;
class AnimatedButton;
struct DropdownOption;   // defined in database.h

class DropdownManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DropdownManagerDialog(QWidget *parent = nullptr);

private slots:
    // Called when the user clicks a different section in the left list.
    // Loads that section's options into the right list.
    void onSectionChanged(int row);

    // Button handlers
    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

    // Called when selection in the options list changes — updates button
    // enabled states (Edit/Delete need a selection; Add always enabled).
    void onOptionSelectionChanged();

private:
    // -----------------------------------------------------------------------
    // Build helpers — splitting construction into smaller methods keeps
    // the code readable and easy to maintain.
    // -----------------------------------------------------------------------
    void buildUi();
    void buildSectionList();    // Populates the left panel
    void buildOptionsList();    // Builds the right panel widgets
    void buildButtons();        // Add/Edit/Delete + Close buttons
    void applyStyles();

    // Loads options for the given section key from SQLite and populates
    // the right-hand list widget.
    void loadOptionsForSection(const QString &sectionKey);

    // Returns the database section key for the currently selected left-list
    // row, e.g. "basis_wet", "scope_dry".
    QString currentSectionKey() const;

    // Returns the human-readable name for the currently selected section,
    // e.g. "Basis Wet" — used in dialog titles and labels.
    QString currentSectionName() const;

    // Updates Edit/Delete button enabled state based on whether an option
    // is selected in the right list.
    void updateButtonStates();

    // -----------------------------------------------------------------------
    // Section data — parallel arrays so we can map list row → key/name.
    // QStringList is a QList<QString>, ideal for small fixed collections.
    // -----------------------------------------------------------------------
    QStringList m_sectionKeys;    // e.g. {"basis_wet", "basis_dry", ...}
    QStringList m_sectionNames;   // e.g. {"Basis Wet", "Basis Dry", ...}

    // Stores the DropdownOption structs for the currently visible section.
    // We need the id field to call Database::deleteOption(id).
    QList<DropdownOption> m_currentOptions;

    // -----------------------------------------------------------------------
    // UI members
    // -----------------------------------------------------------------------
    QListWidget    *m_sectionList   = nullptr;   // Left panel
    QLabel         *m_optionsLabel  = nullptr;   // "OPTIONS FOR: Basis Wet"
    QListWidget    *m_optionsList   = nullptr;   // Right panel

    AnimatedButton *m_addButton     = nullptr;
    AnimatedButton *m_editButton    = nullptr;
    AnimatedButton *m_deleteButton  = nullptr;
    AnimatedButton *m_closeButton   = nullptr;
};

#endif // DROPDOWNMANAGERDIALOG_H