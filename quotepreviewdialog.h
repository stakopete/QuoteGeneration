#ifndef QUOTEPREVIEWDIALOG_H
#define QUOTEPREVIEWDIALOG_H

// ─────────────────────────────────────────────────────────────────────────────
// quotepreviewdialog.h
//
// Quote Preview dialog. Assembles all completed sections into a
// read-only document showing exactly how the final quote will look.
//
// Opens as a separate window so the user can keep editing while
// reviewing the preview.
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>
#include "database.h"

class QTextEdit;
class QLabel;
class AnimatedButton;

class QuotePreviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuotePreviewDialog(const QuoteData &quote,
                                const QList<PriceRow> &priceRows,
                                QWidget *parent = nullptr);

private slots:
    void onClose();
    void onGeneratePdf();  // Saves the current quote to a PDF file


protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void setupUi();
    void buildPreview();
    void onViewPdf();

    // ── Helpers ───────────────────────────────────────────────────────────────
    // Builds an HTML section heading.
    QString sectionHeading(const QString &title) const;

    // Processes tagged text (with [WET] [DRY] tags) into grouped HTML.
    QString processTaggedText(const QString &text,
                              const QString &wetHeading,
                              const QString &dryHeading) const;

    // Formats the price table as HTML.
    QString buildPriceTable() const;

    // ── Controls ──────────────────────────────────────────────────────────────
    QTextEdit      *m_preview;
    QLabel         *m_logoBanner;
    AnimatedButton *m_closeButton;
    AnimatedButton *m_printButton;
    AnimatedButton *m_viewPdfButton;


    // ── Data ──────────────────────────────────────────────────────────────────
    QuoteData        m_quote;
    QList<PriceRow>  m_priceRows;
    AppConfig        m_config;
    QString          m_lastPdfPath;
};

#endif // QUOTEPREVIEWDIALOG_H