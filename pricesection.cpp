// ─────────────────────────────────────────────────────────────────────────────
// pricesection.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "pricesection.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QLocale>
#include "pricesection.h"
#include <QKeyEvent>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
PriceSection::PriceSection(QWidget *parent)
    : QWidget(parent)
{
    m_config = Database::loadConfig();
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// applyGroupBoxStyle()
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::applyGroupBoxStyle(QGroupBox *group)
{
    group->setStyleSheet(StyleManager::instance().groupBoxStyle());
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // ── Section heading ───────────────────────────────────────────────────────
    QLabel *heading = new QLabel("<h3>Proposed Price</h3>");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── Price grid group ──────────────────────────────────────────────────────
    QGroupBox *gridGroup = new QGroupBox("Proposed Price Breakdown");
    applyGroupBoxStyle(gridGroup);
    QVBoxLayout *gridLayout = new QVBoxLayout(gridGroup);

    // ── Table widget ──────────────────────────────────────────────────────────
    // QTableWidget gives us a spreadsheet-like grid.
    // 2 columns: Description and Price.
    m_table = new QTableWidget(0, 2);
    m_table->installEventFilter(this);

    // Set column headers.
    m_table->setHorizontalHeaderLabels({"Description",
                                        "Price (ex " + m_config.taxLabel + ")"});
    // Header is set again after currencySymbol() is available.
    m_table->setHorizontalHeaderItem(1,
                                     new QTableWidgetItem("Price (ex " + m_config.taxLabel + ")"));

    // Description column stretches to fill available width.
    // Price column has a fixed width.
    m_table->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::Stretch
        );
    m_table->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Fixed
        );
    m_table->setColumnWidth(1, 150);

    // Hide the row number column on the left — not needed.
    m_table->verticalHeader()->setVisible(false);

    // Single row selection mode.
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    // Apply styling.
    m_table->setStyleSheet(QString(R"(
        QTableWidget {
            color: %1;
            background-color: %2;
            gridline-color: %3;
            border: 1px solid %3;
            border-radius: 3px;
        }
        QTableWidget::item {
            padding: 4px;
            color: %1;
        }
        QTableWidget::item:selected {
            background-color: %4;
            color: white;
        }
        QHeaderView::section {
            background-color: %5;
            color: white;
            padding: 6px;
            border: none;
            font-weight: bold;
        }
    )").arg(StyleManager::instance().textColour(),
                                    StyleManager::instance().inputBackground(),
                                    StyleManager::instance().borderColour(),
                                    StyleManager::instance().selectionBackground(),
                                    StyleManager::instance().headerBackground()));

    connect(m_table, &QTableWidget::cellChanged,
            this, &PriceSection::onCellChanged);

    gridLayout->addWidget(m_table);

    // ── Add/Remove row buttons ────────────────────────────────────────────────
    m_addRowButton = new AnimatedButton("Add Row");
    m_addRowButton->setFixedSize(138, 50);
    connect(m_addRowButton, &AnimatedButton::clicked,
            this, &PriceSection::onAddRow);

    m_removeRowButton = new AnimatedButton("Remove Row");
    m_removeRowButton->setFixedSize(138, 50);
    connect(m_removeRowButton, &AnimatedButton::clicked,
            this, &PriceSection::onRemoveRow);

    QHBoxLayout *rowButtonLayout = new QHBoxLayout();
    rowButtonLayout->addWidget(m_addRowButton);
    rowButtonLayout->addWidget(m_removeRowButton);
    rowButtonLayout->addStretch();
    gridLayout->addLayout(rowButtonLayout);

    mainLayout->addWidget(gridGroup);

    // ── Totals group ──────────────────────────────────────────────────────────
    QGroupBox *totalsGroup = new QGroupBox("Totals");
    applyGroupBoxStyle(totalsGroup);
    QVBoxLayout *totalsLayout = new QVBoxLayout(totalsGroup);

    // Helper lambda to create a total row with label and value side by side.
    auto makeTotalRow = [&](const QString &labelText,
                            QLabel **valueLabel) -> QHBoxLayout* {
        QHBoxLayout *row = new QHBoxLayout();
        QLabel *lbl = new QLabel(labelText);
        lbl->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; }")
                .arg(StyleManager::instance().labelColour())
            );
        lbl->setFixedWidth(250);

        *valueLabel = new QLabel("$0.00");
        (*valueLabel)->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; }")
                .arg(StyleManager::instance().labelColour())
            );
        (*valueLabel)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        (*valueLabel)->setFixedWidth(150);

        row->addWidget(lbl);
        row->addStretch();
        row->addWidget(*valueLabel);
        return row;
    };

    // Subtotal row.
    totalsLayout->addLayout(
        makeTotalRow("Subtotal (ex " + m_config.taxLabel + "):",
                     &m_subtotalLabel)
        );

    // Tax row — shows the tax label and rate.
    totalsLayout->addLayout(
        makeTotalRow(QString("%1 (%2%):")
                         .arg(m_config.taxLabel)
                         .arg(m_config.taxRate, 0, 'f', 1),
                     &m_taxLabel)
        );

    // Separator line.
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet(
        QString("QFrame { color: %1; }")
            .arg(StyleManager::instance().borderColour())
        );
    totalsLayout->addWidget(line);

    // Total row.
    totalsLayout->addLayout(
        makeTotalRow("Total (inc " + m_config.taxLabel + "):",
                     &m_totalLabel)
        );

    mainLayout->addWidget(totalsGroup);
    mainLayout->addStretch();

    // Add the default 5 rows.
    for (int i = 0; i < 5; ++i)
        addTableRow();
}

// ─────────────────────────────────────────────────────────────────────────────
// addTableRow()
//
// Adds one empty row to the price table.
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::addTableRow()
{
    int row = m_table->rowCount();
    m_table->insertRow(row);

    // Description cell — plain text.
    QTableWidgetItem *descItem = new QTableWidgetItem("");
    m_table->setItem(row, 0, descItem);

    // Price cell — right aligned.
    QTableWidgetItem *priceItem = new QTableWidgetItem(currencySymbol() + "0.00");
    priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_table->setItem(row, 1, priceItem);

    m_table->setRowHeight(row, 32);
}

// ─────────────────────────────────────────────────────────────────────────────
// recalculateTotals()
//
// Reads all price values from the table and recalculates subtotal,
// tax and total. Called whenever a cell value changes.
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::recalculateTotals()
{
    m_subtotal = 0.0;
    QString sym = currencySymbol();

    for (int row = 0; row < m_table->rowCount(); ++row) {
        QTableWidgetItem *priceItem = m_table->item(row, 1);
        if (priceItem) {
            QString text = priceItem->text();
            text.remove(sym).remove(',');
            bool ok;
            double val = text.toDouble(&ok);
            if (ok)
                m_subtotal += val;
        }
    }

    m_tax   = m_subtotal * (m_config.taxRate / 100.0);
    m_total = m_subtotal + m_tax;

    updateTotalLabels();
}

// ─────────────────────────────────────────────────────────────────────────────
// updateTotalLabels()
//
// Formats and displays the calculated totals.
// QLocale formats numbers with correct thousand separators for the
// user's region (e.g. 10,000.00 in Australia).
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::updateTotalLabels()
{
    QLocale locale;
    QString sym = currencySymbol();
    m_subtotalLabel->setText(sym + locale.toString(m_subtotal, 'f', 2));
    m_taxLabel->setText(sym      + locale.toString(m_tax,      'f', 2));
    m_totalLabel->setText(sym    + locale.toString(m_total,    'f', 2));
}

// ─────────────────────────────────────────────────────────────────────────────
// onAddRow()
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::onAddRow()
{
    addTableRow();
}

// ─────────────────────────────────────────────────────────────────────────────
// onRemoveRow()
//
// Removes the currently selected row. If no row is selected, removes
// the last row. Will not remove if only one row remains.
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::onRemoveRow()
{
    if (m_table->rowCount() <= 1) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Cannot Remove");
        msgBox.setText("At least one row must remain in the price schedule.");
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    int row = m_table->currentRow();
    if (row < 0)
        row = m_table->rowCount() - 1;

    m_table->removeRow(row);
    recalculateTotals();
    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// onCellChanged()
//
// Called automatically whenever the user edits any cell.
// We ignore changes during data loading to avoid unnecessary recalculation.
// ─────────────────────────────────────────────────────────────────────────────
void PriceSection::onCellChanged(int row, int column)
{
    if (m_loading)
        return;

    if (column == 1) {
        QTableWidgetItem *item = m_table->item(row, column);
        if (item) {
            m_table->blockSignals(true);

            // Strip any existing currency symbol and commas before parsing.
            QString text = item->text();
            text.remove(currencySymbol()).remove(',');

            bool ok;
            double val = text.toDouble(&ok);
            if (ok) {
                QLocale locale;
                // Add the currency symbol back in front of the formatted value.
                item->setText(currencySymbol() +
                              locale.toString(val, 'f', 2));
            } else {
                // Invalid entry — reset to zero with currency symbol.
                item->setText(currencySymbol() + "0.00");
            }
            item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

            m_table->blockSignals(false);
        }
        recalculateTotals();
    }

    emit dataChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// Data access
// ─────────────────────────────────────────────────────────────────────────────
QList<PriceRow> PriceSection::priceRows() const
{
    QList<PriceRow> rows;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem *descItem  = m_table->item(i, 0);
        QTableWidgetItem *priceItem = m_table->item(i, 1);

        // Only include rows that have at least a description.
        if (descItem && !descItem->text().trimmed().isEmpty()) {
            PriceRow row;
            row.description = descItem->text().trimmed();
            if (priceItem) {
                QString text = priceItem->text();
                text.remove('$').remove(',');
                row.amount = text.toDouble();
            }
            rows.append(row);
        }
    }
    return rows;
}

double PriceSection::subtotal() const { return m_subtotal; }
double PriceSection::taxAmount() const { return m_tax; }
double PriceSection::total() const { return m_total; }

bool PriceSection::isComplete() const
{
    // At least one row must have both a description and a non-zero price.
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QTableWidgetItem *descItem  = m_table->item(i, 0);
        QTableWidgetItem *priceItem = m_table->item(i, 1);

        if (descItem && priceItem &&
            !descItem->text().trimmed().isEmpty()) {
            QString text = priceItem->text();
            text.remove('$').remove(',');
            if (text.toDouble() > 0.0)
                return true;
        }
    }
    return false;
}

void PriceSection::loadData(const QList<PriceRow> &rows)
{
    m_loading = true;

    // Clear existing rows.
    m_table->setRowCount(0);

    // Add loaded rows.
    for (const PriceRow &row : rows) {
        addTableRow();
        int r = m_table->rowCount() - 1;
        m_table->item(r, 0)->setText(row.description);
        m_table->item(r, 1)->setText(QString::number(row.amount, 'f', 2));
    }

    // Ensure at least 5 rows.
    while (m_table->rowCount() < 5)
        addTableRow();

    m_loading = false;
    recalculateTotals();
}

// ─────────────────────────────────────────────────────────────────────────────
// eventFilter()
//
// Intercepts key presses on the table to provide natural navigation.
// Tab moves Description → Price → next row Description.
// Return moves to the next row in the same column.
// Tab on the last price cell creates a new row.
// ─────────────────────────────────────────────────────────────────────────────
bool PriceSection::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_table && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        int row = m_table->currentRow();
        int col = m_table->currentColumn();

        // ── Tab key ───────────────────────────────────────────────────────────
        if (keyEvent->key() == Qt::Key_Tab) {
            if (col == 0) {
                // Description → Price on same row.
                m_table->setCurrentCell(row, 1);
                m_table->editItem(m_table->item(row, 1));
                return true;
            } else if (col == 1) {
                if (row < m_table->rowCount() - 1) {
                    // Price → Description on next row.
                    m_table->setCurrentCell(row + 1, 0);
                    m_table->editItem(m_table->item(row + 1, 0));
                } else {
                    // Last price cell — add new row and move there.
                    addTableRow();
                    int newRow = m_table->rowCount() - 1;
                    m_table->setCurrentCell(newRow, 0);
                    m_table->editItem(m_table->item(newRow, 0));
                }
                return true;
            }
        }

        // ── Return/Enter key ──────────────────────────────────────────────────
        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {

            if (col == 0) {
                // Description → Price on same row (same as Tab).
                m_table->setCurrentCell(row, 1);
                m_table->editItem(m_table->item(row, 1));
                return true;
            } else if (col == 1) {
                if (row < m_table->rowCount() - 1) {
                    // Price → Description on next row.
                    m_table->setCurrentCell(row + 1, 0);
                    m_table->editItem(m_table->item(row + 1, 0));
                } else {
                    // Last price cell — add new row and move there.
                    addTableRow();
                    int newRow = m_table->rowCount() - 1;
                    m_table->setCurrentCell(newRow, 0);
                    m_table->editItem(m_table->item(newRow, 0));
                }
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

// ─────────────────────────────────────────────────────────────────────────────
// currencySymbol()
//
// Returns the correct currency symbol based on the tax configuration.
// GST = Australian dollars ($)
// VAT UK = pounds (£)
// VAT Europe = euros (€)
// Others default to $
// ─────────────────────────────────────────────────────────────────────────────
QString PriceSection::currencySymbol() const
{
    if (m_config.taxLabel == "VAT") {
        // Check hint stored — for now we use a simple heuristic.
        // UK VAT at 20% = pounds, others = euros.
        if (qAbs(m_config.taxRate - 20.0) < 0.01)
            return "£";
        return "€";
    }
    return "$";
}