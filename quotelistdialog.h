#ifndef QUOTELISTDIALOG_H
#define QUOTELISTDIALOG_H

// ─────────────────────────────────────────────────────────────────────────────
// quotelistdialog.h
//
// Quote Management dialog. Shows all saved quotes in a table with
// status, date and site name. Allows the user to:
//   - Open a quote for editing
//   - Change a quote's status
//   - Delete a quote with confirmation
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>
#include "database.h"

class QTableWidget;
class QLabel;
class AnimatedButton;
class QComboBox;

class QuoteListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuoteListDialog(QWidget *parent = nullptr);

    // Returns the ID of the quote the user selected to open.
    // Returns 0 if the user closed without opening a quote.
    int selectedQuoteId() const { return m_selectedQuoteId; }

private slots:
    void onOpenQuote();
    void onDeleteQuote();
    void onChangeStatus();
    void onSelectionChanged();
    void onDoubleClick();

private:
    void setupUi();
    void loadQuotes();
    void applyRowColour(int row, const QString &status);
    int  currentQuoteId() const;

    // ── Controls ──────────────────────────────────────────────────────────────
    QTableWidget   *m_table;
    QLabel         *m_countLabel;
    QComboBox      *m_statusCombo;
    AnimatedButton *m_openButton;
    AnimatedButton *m_changeStatusButton;
    AnimatedButton *m_deleteButton;
    AnimatedButton *m_closeButton;

    // ── Data ──────────────────────────────────────────────────────────────────
    int              m_selectedQuoteId = 0;
    QList<QuoteData> m_quotes;
};

#endif // QUOTELISTDIALOG_H
