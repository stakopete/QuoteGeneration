// ─────────────────────────────────────────────────────────────────────────────
// quotelistdialog.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "quotelistdialog.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QComboBox>
#include <QMessageBox>
#include <QColor>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
QuoteListDialog::QuoteListDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Quote Management");
    resize(850, 500);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setupUi();
    loadQuotes();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void QuoteListDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // ── Heading ───────────────────────────────────────────────────────────────
    QLabel *heading = new QLabel("Quote Management");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Quote count label ─────────────────────────────────────────────────────
    m_countLabel = new QLabel("No quotes found.");
    m_countLabel->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(m_countLabel);

    // ── Table ─────────────────────────────────────────────────────────────────
    // Five columns: ID, Site Name, Date, Status, Last Saved
    m_table = new QTableWidget(0, 5);
    m_table->setHorizontalHeaderLabels({
        "ID", "Site / Project Name", "Date", "Status", "Last Saved"
    });

    // Column widths.
    m_table->setColumnWidth(0, 50);   // ID — narrow
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->setColumnWidth(2, 100);  // Date
    m_table->setColumnWidth(3, 100);  // Status
    m_table->setColumnWidth(4, 140);  // Last Saved

    // Table behaviour.
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);

    // Styling.
    m_table->setStyleSheet(QString(R"(
        QTableWidget {
            color: %1;
            background-color: %2;
            alternate-background-color: %3;
            gridline-color: %4;
            border: 1px solid %4;
            border-radius: 3px;
        }
        QTableWidget::item:selected {
            background-color: %5;
            color: white;
        }
        QHeaderView::section {
            background-color: %6;
            color: white;
            padding: 6px;
            border: none;
            font-weight: bold;
        }
    )").arg(StyleManager::instance().textColour(),
                                    StyleManager::instance().inputBackground(),
                                    StyleManager::instance().panelBackground(),
                                    StyleManager::instance().borderColour(),
                                    StyleManager::instance().selectionBackground(),
                                    StyleManager::instance().headerBackground()));

    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &QuoteListDialog::onSelectionChanged);
    connect(m_table, &QTableWidget::cellDoubleClicked,
            this, [this]() { onDoubleClick(); });

    mainLayout->addWidget(m_table);

    // ── Status change row ─────────────────────────────────────────────────────
    QHBoxLayout *statusRow = new QHBoxLayout();

    QLabel *statusLabel = new QLabel("Change status to:");
    statusLabel->setStyleSheet(
        QString("QLabel { color: %1; }")
            .arg(StyleManager::instance().labelColour())
        );
    statusRow->addWidget(statusLabel);

    m_statusCombo = new QComboBox();
    m_statusCombo->addItems({
        "Draft", "Submitted", "Accepted", "Rejected", "Expired"
    });
    m_statusCombo->setFixedWidth(150);
    m_statusCombo->setStyleSheet(StyleManager::instance().comboBoxStyle());
    statusRow->addWidget(m_statusCombo);

    m_changeStatusButton = new AnimatedButton("Apply Status");
    m_changeStatusButton->setFixedSize(120, 36);
    m_changeStatusButton->setEnabled(false);
    connect(m_changeStatusButton, &AnimatedButton::clicked,
            this, &QuoteListDialog::onChangeStatus);
    statusRow->addWidget(m_changeStatusButton);

    statusRow->addStretch();
    mainLayout->addLayout(statusRow);

    // ── Button row ────────────────────────────────────────────────────────────
    QHBoxLayout *buttonRow = new QHBoxLayout();

    m_closeButton = new AnimatedButton("Close");
    m_closeButton->setFixedSize(100, 36);
    connect(m_closeButton, &AnimatedButton::clicked,
            this, &QDialog::reject);
    buttonRow->addWidget(m_closeButton);

    buttonRow->addStretch();

    m_deleteButton = new AnimatedButton("Delete Quote");
    m_deleteButton->setFixedSize(120, 36);
    m_deleteButton->setEnabled(false);
    connect(m_deleteButton, &AnimatedButton::clicked,
            this, &QuoteListDialog::onDeleteQuote);
    buttonRow->addWidget(m_deleteButton);

    buttonRow->addSpacing(8);

    m_openButton = new AnimatedButton("Open Quote");
    m_openButton->setFixedSize(120, 36);
    m_openButton->setEnabled(false);
    connect(m_openButton, &AnimatedButton::clicked,
            this, &QuoteListDialog::onOpenQuote);
    buttonRow->addWidget(m_openButton);

    mainLayout->addLayout(buttonRow);

    // ── Legend ────────────────────────────────────────────────────────────────
    QLabel *legend = new QLabel(
        "Colour key:  "
        "<span style='background:#e8f5e9; color:#1a1a1a;'>&nbsp;Accepted&nbsp;</span>  "
        "<span style='background:#fff9c4; color:#1a1a1a;'>&nbsp;Submitted&nbsp;</span>  "
        "<span style='background:#ffebee; color:#1a1a1a;'>&nbsp;Rejected&nbsp;</span>  "
        "<span style='background:#ffe0b2; color:#1a1a1a;'>&nbsp;Expired&nbsp;</span>  "
        "<span style='background:#f5f5f5; color:#1a1a1a;'>&nbsp;Draft&nbsp;</span>"
        );
    legend->setTextFormat(Qt::RichText);
    legend->setStyleSheet(
        QString("QLabel { color: %1; font-size: 10px; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(legend);
}

// ─────────────────────────────────────────────────────────────────────────────
// loadQuotes()
//
// Loads all quotes from the database and populates the table.
// Rows are colour coded by status so the user can see at a glance
// which quotes need attention.
// ─────────────────────────────────────────────────────────────────────────────
void QuoteListDialog::loadQuotes()
{
    m_quotes = Database::listQuotes();
    m_table->setRowCount(0);

    if (m_quotes.isEmpty()) {
        m_countLabel->setText("No quotes found.");
        return;
    }

    m_countLabel->setText(
        QString("%1 quote%2 found.")
            .arg(m_quotes.count())
            .arg(m_quotes.count() == 1 ? "" : "s")
        );

    for (const QuoteData &q : m_quotes) {
        int row = m_table->rowCount();
        m_table->insertRow(row);

        // ID column.
        QTableWidgetItem *idItem = new QTableWidgetItem(
            QString::number(q.id)
            );
        idItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 0, idItem);

        // Site name.
        m_table->setItem(row, 1,
                         new QTableWidgetItem(q.siteName));

        // Date.
        QTableWidgetItem *dateItem = new QTableWidgetItem(q.quoteDate);
        dateItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 2, dateItem);

        // Status.
        QTableWidgetItem *statusItem = new QTableWidgetItem(q.status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 3, statusItem);

        // Last saved.
        QTableWidgetItem *savedItem = new QTableWidgetItem(q.lastSaved);
        savedItem->setTextAlignment(Qt::AlignCenter);
        m_table->setItem(row, 4, savedItem);

        // Apply status colour to the row.
        applyRowColour(row, q.status);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// applyRowColour()
//
// Colours each row based on the quote status so the user can see
// at a glance which quotes need attention.
// ─────────────────────────────────────────────────────────────────────────────
void QuoteListDialog::applyRowColour(int row, const QString &status)
{
    QColor colour;
    if      (status == "Accepted")  colour = QColor("#e8f5e9");
    else if (status == "Submitted") colour = QColor("#fff9c4");
    else if (status == "Rejected")  colour = QColor("#ffebee");
    else if (status == "Expired")   colour = QColor("#ffe0b2");
    else                            colour = QColor("#f5f5f5"); // Draft

    for (int col = 0; col < m_table->columnCount(); ++col) {
        QTableWidgetItem *item = m_table->item(row, col);
        if (item)
            item->setBackground(colour);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// currentQuoteId()
//
// Returns the database ID of the currently selected row.
// Returns 0 if nothing is selected.
// ─────────────────────────────────────────────────────────────────────────────
int QuoteListDialog::currentQuoteId() const
{
    int row = m_table->currentRow();
    if (row < 0 || row >= m_quotes.count())
        return 0;
    return m_quotes.at(row).id;
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────

void QuoteListDialog::onSelectionChanged()
{
    bool hasSelection = (m_table->currentRow() >= 0);
    m_openButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
    m_changeStatusButton->setEnabled(hasSelection);
}

void QuoteListDialog::onDoubleClick()
{
    // Double clicking a row is the same as clicking Open.
    onOpenQuote();
}

void QuoteListDialog::onOpenQuote()
{
    int id = currentQuoteId();
    if (id <= 0)
        return;

    m_selectedQuoteId = id;
    accept();   // Close dialog and return Accepted to caller.
}

void QuoteListDialog::onDeleteQuote()
{
    int id = currentQuoteId();
    if (id <= 0)
        return;

    int row = m_table->currentRow();
    QString siteName = m_quotes.at(row).siteName;
    QString status   = m_quotes.at(row).status;

    // Extra warning for accepted quotes.
    QString warningText = QString(
                              "Are you sure you want to permanently delete this quote?\n\n"
                              "Site: %1\nStatus: %2\n\n"
                              "This action cannot be undone."
                              ).arg(siteName, status);

    if (status == "Accepted") {
        warningText += "\n\nWARNING: This quote has been Accepted. "
                       "Are you absolutely sure?";
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Delete Quote");
    msgBox.setText(warningText);
    msgBox.setIcon(QMessageBox::Warning);

    AnimatedButton *yesBtn = new AnimatedButton("Delete", &msgBox);
    yesBtn->setFixedSize(110, 40);
    AnimatedButton *noBtn  = new AnimatedButton("Cancel", &msgBox);
    noBtn->setFixedSize(110, 40);

    msgBox.addButton(yesBtn, QMessageBox::YesRole);
    msgBox.addButton(noBtn,  QMessageBox::NoRole);
    msgBox.setDefaultButton(noBtn);
    msgBox.exec();

    if (msgBox.clickedButton() != yesBtn)
        return;

    if (Database::deleteQuote(id)) {
        loadQuotes();   // Refresh the table.
    } else {
        QMessageBox msgErr(this);
        msgErr.setWindowTitle("Delete Failed");
        msgErr.setText("The quote could not be deleted. "
                       "Please check the database.");
        msgErr.setIcon(QMessageBox::Critical);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgErr);
        okBtn->setFixedSize(110, 40);
        msgErr.addButton(okBtn, QMessageBox::AcceptRole);
        msgErr.exec();
    }
}

void QuoteListDialog::onChangeStatus()
{
    int id = currentQuoteId();
    if (id <= 0)
        return;

    QString newStatus = m_statusCombo->currentText();
    int row = m_table->currentRow();
    QString siteName = m_quotes.at(row).siteName;

    // Confirm the status change.
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Change Status");
    msgBox.setText(
        QString("Change status of:\n\n%1\n\nTo: %2?")
            .arg(siteName, newStatus)
        );
    msgBox.setIcon(QMessageBox::Question);

    AnimatedButton *yesBtn = new AnimatedButton("Yes", &msgBox);
    yesBtn->setFixedSize(110, 40);
    AnimatedButton *noBtn  = new AnimatedButton("No",  &msgBox);
    noBtn->setFixedSize(110, 40);

    msgBox.addButton(yesBtn, QMessageBox::YesRole);
    msgBox.addButton(noBtn,  QMessageBox::NoRole);
    msgBox.exec();

    if (msgBox.clickedButton() != yesBtn)
        return;

    // Load the quote, update its status and save it back.
    QuoteData q = Database::loadQuote(id);
    q.status = newStatus;
    Database::saveQuote(q);

    // Refresh the table to show the new status and colour.
    loadQuotes();
}