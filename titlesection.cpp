// ─────────────────────────────────────────────────────────────────────────────
// titlesection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "titlesection.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include "stylemanager.h"

#include <QDate>
#include <QMessageBox>
#include <QPushButton>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
TitleSection::TitleSection(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void TitleSection::setupUi()
{
    // ── Main layout ───────────────────────────────────────────────────────────
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Quote Title &amp; Date</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Group box ─────────────────────────────────────────────────────────────
    // QGroupBox draws a labelled border around a group of related controls.
    QGroupBox *group = new QGroupBox("Quote Details");
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());

    QFormLayout *form = new QFormLayout(group);
    form->setSpacing(12);
    form->setLabelAlignment(Qt::AlignRight);

    // ── Date field ────────────────────────────────────────────────────────────
    // QDateEdit is a specialised field for entering dates.
    // It shows a calendar popup when the user clicks the arrow.
    m_dateEdit = new QDateEdit();
    m_dateEdit->setDisplayFormat("dd/MM/yyyy");
    m_dateEdit->setDate(QDate::currentDate());  // Default to today
    m_dateEdit->setCalendarPopup(true);         // Show calendar on click
    m_dateEdit->setFixedWidth(150);

    // When the date changes emit our dataChanged signal.
    connect(m_dateEdit, &QDateEdit::dateChanged,
            this, &TitleSection::dataChanged);

    form->addRow("Quote Date:", m_dateEdit);

    // ── Title dropdown ────────────────────────────────────────────────────────
    m_titleCombo = new QComboBox();
    m_titleCombo->setMinimumWidth(400);
    populateTitleDropdown();

    connect(m_titleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TitleSection::onTitleSelectionChanged);

    form->addRow("Quote Title:", m_titleCombo);

    // // ── Custom title field ────────────────────────────────────────────────────
    // // This field is hidden by default and only shown when the user
    // // selects "Other — type your own" from the dropdown.
    // m_customTitle = new QLineEdit();
    // m_customTitle->setPlaceholderText("Type your custom title here...");
    // m_customTitle->setMaxLength(200);
    // m_customTitle->setStyleSheet(
    //     "QLineEdit { "
    //     "color: #2c3e50; "
    //     "background-color: white; "
    //     "border: 1px solid #999999; "
    //     "border-radius: 3px; "
    //     "padding: 3px; }"
    //     );

    // m_customTitle->hide();  // Hidden until needed

    // connect(m_customTitle, &QLineEdit::textChanged,
    //         this, &TitleSection::dataChanged);

    // // ── Add custom title button ───────────────────────────────────────────────
    // // Allows the user to save their custom title to the dropdown
    // // so they can reuse it in future quotes.
    // m_addTitleButton = new QPushButton("Save to List");
    // m_addTitleButton->setFixedWidth(100);
    // m_addTitleButton->hide();  // Hidden until needed
    // m_addTitleButton->setToolTip(
    //     "Save this title to the dropdown list for future use"
    //     );
    // ── Custom title field ────────────────────────────────────────────────────
    m_customTitle = new QLineEdit();
    m_customTitle->setPlaceholderText("Type your custom title here...");
    m_customTitle->setMaxLength(200);

    // Force explicit colours so the field is always visible regardless
    // of what the parent widget's stylesheet says.
    m_customTitle->setStyleSheet(
        "QLineEdit {"
        "    color: #2c3e50;"
        "    background-color: white;"
        "    border: 1px solid #999999;"
        "    border-radius: 3px;"
        "    padding: 4px;"
        "    min-height: 24px;"
        "}"
        );

    // Start hidden — shown only when user picks Other from dropdown.
    m_customTitle->setVisible(false);

    connect(m_customTitle, &QLineEdit::textChanged,
            this, &TitleSection::dataChanged);

    // ── Add custom title button ───────────────────────────────────────────────
    m_addTitleButton = new AnimatedButton("Save to List");
    m_addTitleButton->setFixedWidth(100);
    m_addTitleButton->setVisible(false);
    m_addTitleButton->setToolTip(
        "Save this title to the dropdown list for future use"
        );



    connect(m_addTitleButton, &QPushButton::clicked,
            this, &TitleSection::onAddCustomTitle);

    connect(m_addTitleButton, &QPushButton::clicked,
            this, &TitleSection::onAddCustomTitle);

    // Put the custom title field and button on the same row.
    QHBoxLayout *customRow = new QHBoxLayout();
    customRow->addWidget(m_customTitle);
    customRow->addWidget(m_addTitleButton);
    form->addRow("Custom Title:", customRow);

    // ── Site/Project name ─────────────────────────────────────────────────────
    m_siteName = new QLineEdit();
    m_siteName->setPlaceholderText(
        "Enter the site or project name — this field is required"
        );
    m_siteName->setMaxLength(200);
    m_siteName->setMinimumWidth(400);

    connect(m_siteName, &QLineEdit::textChanged,
            this, &TitleSection::dataChanged);

    form->addRow("Site / Project Name: *", m_siteName);

    mainLayout->addWidget(group);

    // ── Required note ─────────────────────────────────────────────────────────
    QLabel *note = new QLabel(
        "* The Site / Project Name is required before you can preview "
        "or generate the quote."
        );
    note->setStyleSheet("color: #555555; font-size: 11px;");
    note->setWordWrap(true);
    mainLayout->addWidget(note);

    // Push everything to the top — addStretch fills remaining space below.
    mainLayout->addStretch();
}

// ─────────────────────────────────────────────────────────────────────────────
// populateTitleDropdown()
//
// Loads quote title options from the database and adds a custom entry
// at the bottom so the user can type their own.
// ─────────────────────────────────────────────────────────────────────────────
void TitleSection::populateTitleDropdown()
{
    m_titleCombo->clear();

    // Load titles saved in the database.
    QList<DropdownOption> options = Database::loadOptions("quote_title");
    for (const DropdownOption &opt : options) {
        m_titleCombo->addItem(opt.text);
    }

    // Always add an "Other" option at the bottom.
    m_titleCombo->addItem("Other — type your own...");
}

// ─────────────────────────────────────────────────────────────────────────────
// onTitleSelectionChanged()
//
// Shows or hides the custom title field depending on whether the user
// selected "Other" from the dropdown.
// ─────────────────────────────────────────────────────────────────────────────
void TitleSection::onTitleSelectionChanged(int index)
{
    // "Other" is always the last item in the dropdown.
    bool isOther = (index == m_titleCombo->count() - 1);

    m_usingCustomTitle = isOther;
    m_customTitle->setVisible(isOther);
    m_addTitleButton->setVisible(isOther);

    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddCustomTitle()
//
// Saves the custom title to the database so it appears in the dropdown
// next time. Then reloads the dropdown and selects the new entry.
// ─────────────────────────────────────────────────────────────────────────────
void TitleSection::onAddCustomTitle()
{
    QString text = m_customTitle->text().trimmed();

    if (text.isEmpty()) {
        QMessageBox::warning(this, "Empty Title",
                             "Please type a title before saving it to the list.");
        return;
    }

    // Save to database.
    Database::addOption("quote_title", text);

    // Reload the dropdown — the new title will now appear in the list.
    populateTitleDropdown();

    // Select the newly added title.
    int newIndex = m_titleCombo->findText(text);
    if (newIndex >= 0)
        m_titleCombo->setCurrentIndex(newIndex);

    // Hide the custom title field since we're now using the dropdown.
    m_customTitle->hide();
    m_addTitleButton->hide();
    m_usingCustomTitle = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Data access methods
// ─────────────────────────────────────────────────────────────────────────────

QString TitleSection::quoteDate() const
{
    return m_dateEdit->date().toString("dd/MM/yyyy");
}

QString TitleSection::quoteTitle() const
{
    if (m_usingCustomTitle)
        return m_customTitle->text().trimmed();
    return m_titleCombo->currentText();
}

QString TitleSection::siteName() const
{
    return m_siteName->text().trimmed();
}

bool TitleSection::isComplete() const
{
    // Site name is the only mandatory field in this section.
    // Date always has a value (defaults to today).
    // Title always has a value (defaults to first dropdown item).
    return !m_siteName->text().trimmed().isEmpty();
}

void TitleSection::loadData(const QString &date,
                            const QString &title,
                            const QString &site)
{
    // Parse and set the date.
    QDate d = QDate::fromString(date, "dd/MM/yyyy");
    if (d.isValid())
        m_dateEdit->setDate(d);

    // Try to find the title in the dropdown.
    int index = m_titleCombo->findText(title);
    if (index >= 0) {
        m_titleCombo->setCurrentIndex(index);
    } else {
        // Not found — select Other and put the text in the custom field.
        m_titleCombo->setCurrentIndex(m_titleCombo->count() - 1);
        m_customTitle->setText(title);
    }

    m_siteName->setText(site);
}