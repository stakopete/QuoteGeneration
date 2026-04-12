#ifndef PRICESECTION_H
#define PRICESECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// pricesection.h
//
// Proposed Price section. Presents a grid of description/price rows
// with automatic subtotal, tax, and total calculation.
//
// The price grid starts with 5 rows. Additional rows can be added
// via the Add Row button.
//
// Tax label and rate come from AppConfig so they reflect whatever
// the user configured (GST, VAT etc.)
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"
#include <QEvent>

class QTableWidget;
class QLabel;
class QGroupBox;
class AnimatedButton;

// Represents one row in the price grid.
struct PriceRow {
    QString description;
    double  amount = 0.0;
};

class PriceSection : public QWidget
{
    Q_OBJECT

public:
    explicit PriceSection(QWidget *parent = nullptr);

    // Returns all price rows that have data in them.
    QList<PriceRow> priceRows() const;

    // Returns the subtotal (sum of all amounts).
    double subtotal() const;

    // Returns the tax amount.
    double taxAmount() const;

    // Returns the total including tax.
    double total() const;

    // Returns true if at least one row has both description and price.
    bool isComplete() const;

    // Loads previously saved price rows.
    void loadData(const QList<PriceRow> &rows);

signals:
    void dataChanged();

private slots:
    void onAddRow();
    void onRemoveRow();
    void onCellChanged(int row, int column);

private:
    void setupUi();
    void applyGroupBoxStyle(QGroupBox *group);
    void addTableRow();
    void recalculateTotals();
    void updateTotalLabels();

    // ── Controls ──────────────────────────────────────────────────────────────
    QTableWidget   *m_table;
    QLabel         *m_subtotalLabel;
    QLabel         *m_taxLabel;
    QLabel         *m_totalLabel;
    AnimatedButton *m_addRowButton;
    AnimatedButton *m_removeRowButton;

    // ── Data ──────────────────────────────────────────────────────────────────
    AppConfig       m_config;       // For tax label and rate
    double          m_subtotal = 0.0;
    double          m_tax      = 0.0;
    double          m_total    = 0.0;

    QString currencySymbol() const;

    // Flag to prevent recalculation triggering while we are loading data.
    bool            m_loading  = false;
    bool            eventFilter(QObject *obj, QEvent *event) override;
};

#endif // PRICESECTION_H