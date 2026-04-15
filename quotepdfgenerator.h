#ifndef QUOTEPDFGENERATOR_H
#define QUOTEPDFGENERATOR_H

// ─────────────────────────────────────────────────────────────────────────────
// quotepdfgenerator.h
//
// Responsible for one thing only: taking a complete quote's data and
// writing it to a PDF file on disk.
//
// It is deliberately kept separate from QuotePreviewDialog so that:
//   - The preview dialog handles display (screen)
//   - This class handles output (file)
// Each class has a single, clear responsibility.
//
// Usage:
//   QuotePdfGenerator gen(quoteData, priceRows, appConfig);
//   bool ok = gen.generate("C:/Quotes/Quote_001.pdf");
// ─────────────────────────────────────────────────────────────────────────────

#include <QString>
#include <QList>
#include "database.h"   // for QuoteData, AppConfig, PriceRow

class QuotePdfGenerator
{
public:
    // Constructor — stores references to the data we need.
    // All three are passed in from outside; this class does not touch
    // the database or the UI directly.
    QuotePdfGenerator(const QuoteData    &quote,
                      const QList<PriceRow> &priceRows,
                      const AppConfig    &config);

    // The main entry point.
    // Renders the quote to a PDF at the given filePath.
    // Returns true on success, false if the file could not be written.
    bool generate(const QString &filePath);



private:
    // ── Data ──────────────────────────────────────────────────────────────────
    // Copies of the data passed in at construction time.
    // We take copies (not pointers) so the generator is self-contained
    // and safe to use even if the caller's data changes.
    QuoteData        m_quote;
    QList<PriceRow>  m_priceRows;
    AppConfig        m_config;

    // ── Page geometry ─────────────────────────────────────────────────────────
    // The usable page width in pixels at ScreenResolution.
    // Calculated once in generate() from the real QPrinter object so it
    // reflects the actual margins — not a hardcoded guess.
    // Declared mutable so the const builder methods can read it.
    mutable int m_pageWidthPx = 566;

    // ── Private HTML builders ─────────────────────────────────────────────────
    QString buildHtml()          const;
    QString buildHeaderHtml()    const;
    QString buildTitleBlockHtml() const;
    QString buildSectionHtml(const QString &heading,
                             const QString &body) const;
    QString buildPriceTableHtml() const;
    QString buildSignatureHtml() const;
};
#endif // QUOTEPDFGENERATOR_H