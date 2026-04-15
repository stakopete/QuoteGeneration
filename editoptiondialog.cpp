// ---------------------------------------------------------------------------
// editoptiondialog.cpp
// ---------------------------------------------------------------------------
// Implementation of the small Add/Edit text entry dialog.
//
// Design decisions:
//   - Uses AnimatedButton for OK/Cancel to match the rest of the application.
//   - Validates that the text field is not blank before accepting — this
//     prevents empty strings entering the database.
//   - Applies StyleManager colours so it honours the current light/dark mode.
// ---------------------------------------------------------------------------

#include "editoptiondialog.h"

#include "animatedbutton.h"   // Our custom button with press animation
#include "stylemanager.h"     // Centralised colour/stylesheet access

#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDialogButtonBox>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
EditOptionDialog::EditOptionDialog(QWidget *parent,
                                   const QString &title,
                                   const QString &label,
                                   const QString &initialText)
    : QDialog(parent)
{
    // Remove the "?" help button that Qt adds to dialogs by default —
    // it serves no purpose here and clutters the title bar.
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    setWindowTitle(title);

    // Fix the dialog to a sensible width — it only needs one line of input.
    setFixedWidth(420);

    buildUi(label, initialText);
    applyStyles();
}

// ---------------------------------------------------------------------------
// text()
// Returns the trimmed content of the line edit.
// Trimmed = leading/trailing whitespace removed, which keeps the database
// clean without requiring the user to be precise.
// ---------------------------------------------------------------------------
QString EditOptionDialog::text() const
{
    return m_lineEdit->text().trimmed();
}

// ---------------------------------------------------------------------------
// buildUi()
// Constructs all widgets and lays them out.
// ---------------------------------------------------------------------------
void EditOptionDialog::buildUi(const QString &label, const QString &initialText)
{
    // -- Main vertical layout ------------------------------------------------
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    // -- Descriptive label ---------------------------------------------------
    // Tells the user what section they're adding/editing for, e.g.
    // "Enter option text for: Basis Wet"
    auto *descLabel = new QLabel(label, this);
    descLabel->setWordWrap(true);   // In case the section name is long
    mainLayout->addWidget(descLabel);

    // -- Text input field ----------------------------------------------------
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setText(initialText);           // Pre-fill for Edit mode
    m_lineEdit->setPlaceholderText("Enter option text...");

    // Select all existing text so the user can type over it immediately
    // without needing to manually select/clear — good UX for Edit mode.
    m_lineEdit->selectAll();

    // Allow pressing Enter to trigger OK — feels natural for a single field.
    // We connect returnPressed to our onOkClicked slot (which validates first).
    connect(m_lineEdit, &QLineEdit::returnPressed,
            this, &EditOptionDialog::onOkClicked);

    mainLayout->addWidget(m_lineEdit);

    // -- Spacer between input and buttons ------------------------------------
    mainLayout->addSpacing(8);

    // -- Button row ----------------------------------------------------------
    // AnimatedButton takes: (normal image, pressed image, parent)
    // We pass empty strings here because AnimatedButton should fall back to
    // text label + StyleManager stylesheet when no image paths are provided.
    // Adjust the image paths to match your project's actual button assets.
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);

    // Stretch on the left pushes buttons to the right — standard dialog layout.
    buttonLayout->addStretch();

    m_okButton     = new AnimatedButton("OK",     this);

    m_okButton->setFixedSize(100, 36);
    connect(m_okButton, &AnimatedButton::clicked,
            this, &EditOptionDialog::onOkClicked);
    buttonLayout->addWidget(m_okButton);

    m_cancelButton = new AnimatedButton("Cancel", this);

    m_cancelButton->setFixedSize(100, 36);
    connect(m_cancelButton, &AnimatedButton::clicked,
            this, &QDialog::reject);   // reject() closes with Rejected result
    buttonLayout->addWidget(m_cancelButton);

    mainLayout->addLayout(buttonLayout);

    // Give focus to the line edit immediately so the user can type without
    // needing to click first.
    m_lineEdit->setFocus();
}

// ---------------------------------------------------------------------------
// applyStyles()
// Asks StyleManager for the current palette and applies it consistently.
// ---------------------------------------------------------------------------
void EditOptionDialog::applyStyles()
{
    auto &sm = StyleManager::instance();

    // Apply the application-wide stylesheet to this dialog so backgrounds,
    // fonts and widget colours all match the rest of the application.
    setStyleSheet(sm.groupBoxStyle()  +
                  sm.listWidgetStyle() +
                  sm.lineEditStyle()   +
                  sm.comboBoxStyle());
}

// ---------------------------------------------------------------------------
// onOkClicked()  [private slot]
// Validates the input before accepting — prevents empty entries in the DB.
// ---------------------------------------------------------------------------
void EditOptionDialog::onOkClicked()
{
    if (m_lineEdit->text().trimmed().isEmpty())
    {
        // Warn the user rather than silently ignoring the click.
        QMessageBox::warning(this,
                             "Empty Text",
                             "Please enter some text for the option.\n"
                             "The option text cannot be blank.");

        // Return focus to the field so the user can type immediately.
        m_lineEdit->setFocus();
        return;
    }

    // All good — close the dialog with an Accepted result code.
    // The caller checks exec() == QDialog::Accepted before reading text().
    accept();
}