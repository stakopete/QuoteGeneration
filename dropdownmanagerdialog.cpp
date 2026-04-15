// ---------------------------------------------------------------------------
// dropdownmanagerdialog.cpp
// ---------------------------------------------------------------------------
// Full implementation of the Dropdown Manager settings dialog.
//
// Key design decisions:
//   - Two-panel layout using QSplitter so the user can resize if needed.
//   - Section list is static (hard-coded) — the sections are defined by the
//     application's data model, not by the database.
//   - Option list is dynamic — loaded fresh from SQLite each time the user
//     selects a section, so it always reflects current database state.
//   - Edit is implemented as delete-then-add because Database only exposes
//     addOption() and deleteOption() (no updateOption). This is simpler and
//     perfectly correct for our use case.
//   - All colours from StyleManager so light/dark mode works automatically.
// ---------------------------------------------------------------------------

#include "dropdownmanagerdialog.h"

#include "editoptiondialog.h"   // Our helper dialog for Add/Edit text entry
#include "database.h"           // Database::loadOptions / addOption / deleteOption
#include "animatedbutton.h"     // Custom animated button
#include "stylemanager.h"       // Centralised colours and stylesheets

#include <QListWidget>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QFrame>
#include <QFont>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
DropdownManagerDialog::DropdownManagerDialog(QWidget *parent)
    : QDialog(parent)
{
    // Remove the "?" help button from the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle("Dropdown Option Manager");

    // Give the dialog a comfortable default size.
    // The splitter lets the user adjust, but this is a good starting ratio.
    resize(760, 520);

    // Populate the section key and name lists before building the UI,
    // because buildSectionList() reads from them.
    //
    // IMPORTANT: The keys here must exactly match those used when options
    // were saved to the database (i.e. the section keys your application
    // passes to Database::addOption / loadOptions).
    m_sectionKeys  << "basis_wet"      << "basis_dry"
                  << "scope_wet"      << "scope_dry"
                  << "exclusions_wet" << "exclusions_dry"
                  << "general_wet"    << "general_dry"
                  << "quote_title";

    m_sectionNames << "Basis Wet"       << "Basis Dry"
                   << "Scope Wet"       << "Scope Dry"
                   << "Exclusions Wet"  << "Exclusions Dry"
                   << "General Wet"     << "General Dry"
                   << "Quote Title";

    buildUi();
    applyStyles();

    // Select the first section automatically so the right panel shows
    // something useful immediately rather than being blank on first open.
    m_sectionList->setCurrentRow(0);
}

// ===========================================================================
// UI CONSTRUCTION
// ===========================================================================

// ---------------------------------------------------------------------------
// buildUi()
// Top-level layout assembly. Calls helpers for each region.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::buildUi()
{
    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(12, 12, 12, 12);
    outerLayout->setSpacing(10);

    // -- Title label ---------------------------------------------------------
    auto *titleLabel = new QLabel("Manage Dropdown Options", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(13);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    outerLayout->addWidget(titleLabel);

    // -- Horizontal rule beneath title ---------------------------------------
    auto *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    outerLayout->addWidget(line);

    // -- Splitter holding left + right panels --------------------------------
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setChildrenCollapsible(false);

    buildSectionList();
    splitter->addWidget(m_sectionList);

    buildOptionsList();
    buildButtons();

    auto *rightWidget = new QWidget(this);
    auto *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(8, 0, 0, 0);
    rightLayout->setSpacing(8);
    rightLayout->addWidget(m_optionsLabel);
    rightLayout->addWidget(m_optionsList);
    splitter->addWidget(rightWidget);

    splitter->setSizes({240, 500});

    // Splitter added before button row so it fills the middle.
    outerLayout->addWidget(splitter, 1);

    // -- Single button row at the bottom — Close | gap | Add Edit Delete ----
    auto *buttonRow = new QHBoxLayout();
    buttonRow->addWidget(m_closeButton);
    buttonRow->addStretch();
    buttonRow->addWidget(m_addButton);
    buttonRow->addSpacing(8);
    buttonRow->addWidget(m_editButton);
    buttonRow->addSpacing(8);
    buttonRow->addWidget(m_deleteButton);
    outerLayout->addLayout(buttonRow);
}

// ---------------------------------------------------------------------------
// buildSectionList()
// Creates the left-hand QListWidget and populates it with section names.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::buildSectionList()
{
    m_sectionList = new QListWidget(this);

    // Populate from our parallel arrays (filled in the constructor).
    for (const QString &name : std::as_const(m_sectionNames))
    {
        m_sectionList->addItem(name);
    }

    // When the user clicks a section, load its options.
    // currentRowChanged gives us the new row index (or -1 if selection cleared).
    connect(m_sectionList, &QListWidget::currentRowChanged,
            this, &DropdownManagerDialog::onSectionChanged);
}

// ---------------------------------------------------------------------------
// buildOptionsList()
// Creates the right-hand label and QListWidget for displaying options.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::buildOptionsList()
{
    // This label updates to show which section is currently displayed.
    m_optionsLabel = new QLabel("Options:", this);
    QFont f = m_optionsLabel->font();
    f.setBold(true);
    m_optionsLabel->setFont(f);

    m_optionsList = new QListWidget(this);

    // When the selection in the options list changes, update Edit/Delete state.
    connect(m_optionsList, &QListWidget::itemSelectionChanged,
            this, &DropdownManagerDialog::onOptionSelectionChanged);
}

// ---------------------------------------------------------------------------
// buildButtons()
// Creates all four AnimatedButtons with appropriate sizes.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::buildButtons()
{
    // AnimatedButton signature: (normalImagePath, pressedImagePath, parent)
    // Empty strings = no border-image; the button falls back to text + style.
    m_addButton    = new AnimatedButton("Add",    this);
    m_editButton   = new AnimatedButton("Edit",   this);
    m_deleteButton = new AnimatedButton("Delete", this);
    m_closeButton  = new AnimatedButton("Close",  this);

    // Uniform button sizing for a tidy layout
    const QSize btnSize(90, 34);
    m_addButton->setFixedSize(btnSize);
    m_editButton->setFixedSize(btnSize);
    m_deleteButton->setFixedSize(btnSize);
    m_closeButton->setFixedSize(110, 34);

    // Wire up the signals
    connect(m_addButton,    &AnimatedButton::clicked,
            this, &DropdownManagerDialog::onAddClicked);

    connect(m_editButton,   &AnimatedButton::clicked,
            this, &DropdownManagerDialog::onEditClicked);

    connect(m_deleteButton, &AnimatedButton::clicked,
            this, &DropdownManagerDialog::onDeleteClicked);

    // Close simply rejects the dialog (no data changes pending — everything
    // is committed to SQLite immediately so no "Cancel" semantics needed).
    connect(m_closeButton,  &AnimatedButton::clicked,
            this, &QDialog::accept);

    // Initial state: Edit/Delete disabled until the user selects an option.
    updateButtonStates();
}

// ---------------------------------------------------------------------------
// applyStyles()
// ---------------------------------------------------------------------------
void DropdownManagerDialog::applyStyles()
{
    auto &sm = StyleManager::instance();

    // Apply the global stylesheet to inherit all themed widget styles.
    setStyleSheet(sm.groupBoxStyle()  +
                  sm.listWidgetStyle() +
                  sm.lineEditStyle()   +
                  sm.comboBoxStyle());

    // Style the section list to visually distinguish it from the right pane.
    // We give selected items a highlight colour from StyleManager.
    const QString sectionListStyle = QString(
                                         "QListWidget {"
                                         "   border: 1px solid %1;"
                                         "   border-radius: 4px;"
                                         "   padding: 2px;"
                                         "   font-size: 12px;"
                                         "}"
                                         "QListWidget::item {"
                                         "   padding: 6px 8px;"
                                         "   border-radius: 3px;"
                                         "}"
                                         "QListWidget::item:selected {"
                                         "   background-color: %2;"
                                         "   color: %3;"
                                         "}"
                                         "QListWidget::item:hover:!selected {"
                                         "   background-color: %4;"
                                         "}"
                                         ).arg(
                                             sm.borderColour(),         // border colour
                                             sm.selectionBackground(),  // selected background
                                             sm.selectionText(),        // selected text
                                             sm.inputBackground()       // hover background
                                             );

    m_sectionList->setStyleSheet(sectionListStyle);

    // Options list gets the same treatment for consistency.
    m_optionsList->setStyleSheet(sectionListStyle);
}

// ===========================================================================
// DATA LOADING
// ===========================================================================

// ---------------------------------------------------------------------------
// loadOptionsForSection()
// Queries the database and populates the right-hand list.
// Called whenever the section selection changes.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::loadOptionsForSection(const QString &sectionKey)
{
    // Clear the list widget and our cached options before re-populating.
    m_optionsList->clear();
    m_currentOptions.clear();

    // Ask the Database class for all options in this section.
    // loadOptions() returns QList<DropdownOption> where each item has:
    //   int     id       — primary key (needed for deleteOption)
    //   QString text     — the display text
    m_currentOptions = Database::loadOptions(sectionKey);

    // Add each option to the visible list widget.
    for (const DropdownOption &opt : std::as_const(m_currentOptions))
    {
        m_optionsList->addItem(opt.text);
    }

    // After loading, no item is selected, so disable Edit/Delete.
    updateButtonStates();
}

// ===========================================================================
// HELPER METHODS
// ===========================================================================

// ---------------------------------------------------------------------------
// currentSectionKey()
// Returns the database key for the currently selected section row.
// ---------------------------------------------------------------------------
QString DropdownManagerDialog::currentSectionKey() const
{
    int row = m_sectionList->currentRow();

    // Guard against -1 (nothing selected) — shouldn't happen in normal use
    // because we select row 0 in the constructor, but defensive is good.
    if (row < 0 || row >= m_sectionKeys.size())
        return QString();

    return m_sectionKeys.at(row);
}

// ---------------------------------------------------------------------------
// currentSectionName()
// Returns the human-readable name for the currently selected section.
// ---------------------------------------------------------------------------
QString DropdownManagerDialog::currentSectionName() const
{
    int row = m_sectionList->currentRow();

    if (row < 0 || row >= m_sectionNames.size())
        return QString();

    return m_sectionNames.at(row);
}

// ---------------------------------------------------------------------------
// updateButtonStates()
// Enables or disables Edit/Delete based on whether an option is selected.
// Add is always enabled as long as a section is selected.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::updateButtonStates()
{
    bool hasSelection  = (m_optionsList->currentRow() >= 0);
    bool hasSection    = (m_sectionList->currentRow() >= 0);

    m_addButton->setEnabled(hasSection);
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

// ===========================================================================
// SLOTS
// ===========================================================================

// ---------------------------------------------------------------------------
// onSectionChanged()
// User clicked a section in the left list — reload the right list.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::onSectionChanged(int row)
{
    if (row < 0)
        return;

    // Update the right-panel header label to show which section is active.
    m_optionsLabel->setText(QString("Options for: %1").arg(m_sectionNames.at(row)));

    // Load this section's options from the database.
    loadOptionsForSection(m_sectionKeys.at(row));
}

// ---------------------------------------------------------------------------
// onAddClicked()
// Opens EditOptionDialog in "Add" mode and inserts result into the database.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::onAddClicked()
{
    const QString sectionName = currentSectionName();
    const QString sectionKey  = currentSectionKey();

    if (sectionKey.isEmpty())
        return;

    // Open the entry dialog.  Empty initialText = blank field (Add mode).
    EditOptionDialog dlg(this,
                         "Add Option",
                         QString("Enter new option text for: %1").arg(sectionName));

    if (dlg.exec() != QDialog::Accepted)
        return;   // User cancelled — do nothing

    const QString newText = dlg.text();

    // Persist to the database.
    if (!Database::addOption(sectionKey, newText))
    {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to save the new option.\n"
                              "Please check the database and try again.");
        return;
    }

    // Reload the list from the database so the new entry appears.
    // This is simpler and safer than manually inserting into m_currentOptions.
    loadOptionsForSection(sectionKey);

    // Select the newly added item — it will be the last one in the list,
    // because SQLite returns rows in insert order.
    int lastRow = m_optionsList->count() - 1;
    if (lastRow >= 0)
        m_optionsList->setCurrentRow(lastRow);
}

// ---------------------------------------------------------------------------
// onEditClicked()
// Opens EditOptionDialog pre-filled with the selected option's text.
// Saves changes as delete-then-add (our Database API has no updateOption).
// ---------------------------------------------------------------------------
void DropdownManagerDialog::onEditClicked()
{
    int optRow = m_optionsList->currentRow();

    if (optRow < 0 || optRow >= m_currentOptions.size())
        return;

    const DropdownOption &selected = m_currentOptions.at(optRow);
    const QString sectionKey       = currentSectionKey();
    const QString sectionName      = currentSectionName();

    // Open the entry dialog pre-filled with the existing text.
    EditOptionDialog dlg(this,
                         "Edit Option",
                         QString("Edit option text for: %1").arg(sectionName),
                         selected.text);   // <-- pre-fills the field

    if (dlg.exec() != QDialog::Accepted)
        return;

    const QString updatedText = dlg.text();

    // If the user didn't actually change anything, skip the database round-trip.
    if (updatedText == selected.text)
        return;

    // -- Delete the old record -----------------------------------------------
    if (!Database::deleteOption(selected.id))
    {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to remove the old option.\n"
                              "The edit has been cancelled.");
        return;
    }

    // -- Insert the new text -------------------------------------------------
    if (!Database::addOption(sectionKey, updatedText))
    {
        QMessageBox::critical(this,
                              "Database Error",
                              "The old option was removed but the updated text\n"
                              "could not be saved. Please add it manually.");
        return;
    }

    // Reload and try to re-select the same position so the UX feels stable.
    loadOptionsForSection(sectionKey);

    // Clamp the row in case the list shrank (shouldn't happen here, but safe).
    int newRow = qMin(optRow, m_optionsList->count() - 1);
    if (newRow >= 0)
        m_optionsList->setCurrentRow(newRow);
}

// ---------------------------------------------------------------------------
// onDeleteClicked()
// Asks for confirmation then removes the selected option from the database.
// ---------------------------------------------------------------------------
void DropdownManagerDialog::onDeleteClicked()
{
    int optRow = m_optionsList->currentRow();

    if (optRow < 0 || optRow >= m_currentOptions.size())
        return;

    const DropdownOption &selected = m_currentOptions.at(optRow);

    // Always confirm destructive operations — the user may have mis-clicked.
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm Delete");
    msgBox.setText(QString("Are you sure you want to delete:\n\n"
                           "\"%1\"\n\nThis cannot be undone.")
                       .arg(selected.text));
    msgBox.setIcon(QMessageBox::Question);

    AnimatedButton *yesBtn = new AnimatedButton("Yes", &msgBox);
    yesBtn->setFixedSize(110, 40);
    AnimatedButton *noBtn  = new AnimatedButton("No",  &msgBox);
    noBtn->setFixedSize(110, 40);

    msgBox.addButton(yesBtn, QMessageBox::YesRole);
    msgBox.addButton(noBtn,  QMessageBox::NoRole);
    msgBox.setDefaultButton(noBtn);
    msgBox.exec();

    if (msgBox.clickedButton() != yesBtn)
        return;

    // Delete from the database using the option's primary key id.
    if (!Database::deleteOption(selected.id))
    {
        QMessageBox::critical(this,
                              "Database Error",
                              "Failed to delete the option.\n"
                              "Please check the database and try again.");
        return;
    }

    // Reload the list.
    const QString sectionKey = currentSectionKey();
    loadOptionsForSection(sectionKey);

    // Try to keep the selection near where it was so the user can quickly
    // delete multiple items without having to reselect each time.
    int newRow = qMin(optRow, m_optionsList->count() - 1);
    if (newRow >= 0)
        m_optionsList->setCurrentRow(newRow);
}

// ---------------------------------------------------------------------------
// onOptionSelectionChanged()
// Called when the selection in the options list changes.
// Just delegates to updateButtonStates().
// ---------------------------------------------------------------------------
void DropdownManagerDialog::onOptionSelectionChanged()
{
    updateButtonStates();
}