// ─────────────────────────────────────────────────────────────────────────────
// quotepdfgenerator.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "quotepdfgenerator.h"

#include <QTextDocument>
#include <QPrinter>
#include <QLocale>
#include <QFileInfo>
#include <QDir>
#include <QDate>
#include <QImage>
#include <QBuffer>
#include <QByteArray>
//#include <QDebug>


// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
QuotePdfGenerator::QuotePdfGenerator(const QuoteData       &quote,
                                     const QList<PriceRow> &priceRows,
                                     const AppConfig       &config)
    : m_quote(quote)
    , m_priceRows(priceRows)
    , m_config(config)
    , m_pageWidthPx(566)   // Safe default — overwritten in generate() below.
{
}

// ─────────────────────────────────────────────────────────────────────────────
// generate()
//
// CRITICAL — order of operations:
//
//   Old code called buildHtml() FIRST, then configured the printer.
//   That meant m_pageWidthPx was still 566 (the default) when the HTML
//   builders ran, so the banner and table used the wrong width.
//
//   Correct order:
//     1. Configure QPrinter fully.
//     2. Read the real printable pixel width from the printer.
//     3. Build the HTML — builders now see the correct m_pageWidthPx.
//     4. Load HTML into QTextDocument.
//     5. Print.
// ─────────────────────────────────────────────────────────────────────────────
bool QuotePdfGenerator::generate(const QString &filePath)
{
    // ── 1. Configure printer ──────────────────────────────────────────────────
    // ScreenResolution avoids the DPI mismatch that makes text tiny.
    QPrinter printer(QPrinter::ScreenResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setPageMargins(QMarginsF(15, 15, 15, 15), QPageLayout::Millimeter);

    // ── 2. Measure the real printable width ───────────────────────────────────
    // DevicePixel gives us screen pixels after margins are subtracted.
    // This is the unit our HTML img width= and table width= attributes use.
    // Use Point width for HTML layout.
    // QTextDocument renders HTML in points, so width values in img and table
    // tags are interpreted as points, not screen pixels.
    // The page size is also set in Points below, so these units must match.
    m_pageWidthPx = printer.pageRect(QPrinter::Point).width();


    // ── 3. Build HTML — m_pageWidthPx is now correct ──────────────────────────
    QString html = buildHtml();

    // ── 4. Load into QTextDocument ────────────────────────────────────────────
    QTextDocument doc;
    doc.setHtml(html);
    // Page size in Points (typographic units) so text layout matches the page.
    doc.setPageSize(QSizeF(printer.pageRect(QPrinter::Point).size()));
    QFont baseFont("Times New Roman", 10);
    doc.setDefaultFont(baseFont);

    // ── 5. Print to PDF ───────────────────────────────────────────────────────
    doc.print(&printer);

    QFileInfo fi(filePath);
    return fi.exists() && fi.size() > 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildHtml()
//
// Assembles the complete document. The <style> block is kept only for body
// defaults. All heading and element sizes use inline style= attributes because
// QTextDocument does not reliably apply <style> block rules to block elements.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePdfGenerator::buildHtml() const
{
    QString html;

    html += R"(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<style>
  body {
      font-family: 'Times New Roman', Times, serif;
      font-size: 9pt;
      color: #1a1a1a;
      margin: 0;
      padding: 0;
  }
  p {
      margin: 2px 0;
      line-height: 1.4;
  }
</style>
</head>
<body>
)";

    html += buildHeaderHtml();
    html += buildTitleBlockHtml();

    if (!m_quote.systemText.trimmed().isEmpty())
        html += buildSectionHtml("System Offered", m_quote.systemText);

    if (!m_quote.basisText.trimmed().isEmpty())
        html += buildSectionHtml("Basis of Design", m_quote.basisText);

    if (!m_quote.scopeText.trimmed().isEmpty())
        html += buildSectionHtml("Scope of Works", m_quote.scopeText);

    html += buildPriceTableHtml();

    if (!m_quote.exclusions.trimmed().isEmpty())
        html += buildSectionHtml("Exclusions", m_quote.exclusions);

    if (!m_quote.generalConditions.trimmed().isEmpty())
        html += buildSectionHtml("General Conditions", m_quote.generalConditions);

    if (!m_quote.clarifications.trimmed().isEmpty())
        html += buildSectionHtml("Clarifications", m_quote.clarifications);

    html += buildSignatureHtml();

    html += "</body></html>";
    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildHeaderHtml()
//
// Logo banner scaled to exactly the printable page width.
//
// Two key facts about QTextDocument HTML rendering:
//   - It cannot load images from file paths — must use Base64 data URI.
//   - It ignores width="100%" on <img> — must use an explicit pixel value.
//
// m_pageWidthPx was measured from the real printer in generate(), so the
// image will fill the page precisely without cutting off or leaving a gap.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePdfGenerator::buildHeaderHtml() const
{
    if (m_config.logoPath.isEmpty())
        return QString();

    QImage img(m_config.logoPath);
    if (img.isNull())
        return QString();

    // Scale to the exact printable width.
    img = img.scaledToWidth(m_pageWidthPx, Qt::SmoothTransformation);

    // Encode as Base64 PNG for embedding in the HTML string.
    QByteArray imageData;
    QBuffer    buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    QString b64 = QString::fromLatin1(imageData.toBase64());

    QString html;
    html += QString("<img src='data:image/png;base64,%1' width='%2' />")
                .arg(b64)
                .arg(m_pageWidthPx);
    html += "<p style='margin:6px 0;'></p>";

    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildTitleBlockHtml()
//
// Quote Title (11pt, centred, plain) then Site/Project Name (12pt, centred,
// bold), then the reference/date table.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePdfGenerator::buildTitleBlockHtml() const
{
    // Builds one label/value row in the info table.
    auto row = [](const QString &label, const QString &value) -> QString {
        return QString(
                   "<tr>"
                   "<td style='width:25%; font-size:9pt; font-weight:bold;"
                   " padding:2px 6px 2px 0;'>%1</td>"
                   "<td style='font-size:9pt; padding:2px 0;'>%2</td>"
                   "</tr>"
                   ).arg(label, value.toHtmlEscaped());
    };

    // Quote Reference: YYMMnnn  e.g. 2603001
    QString quoteRef;
    if (m_quote.id > 0) {
        QDate   today = QDate::currentDate();
        QString yy    = today.toString("yy");
        QString mm    = today.toString("MM");
        QString seq   = QString("%1").arg(m_quote.id, 3, 10, QChar('0'));
        quoteRef = yy + mm + seq;
    } else {
        quoteRef = "Draft";
    }

    QString html;

    // Quote Title — 11pt, centred, not bold.
    if (!m_quote.titleText.trimmed().isEmpty()) {
        html += QString(
                    "<p style='font-size:11pt; text-align:center;"
                    " color:#1a1a1a; margin-bottom:4px;'>%1</p>"
                    ).arg(m_quote.titleText.toHtmlEscaped());
    }

    // Site/Project Name — 12pt, centred, bold.
    html += QString(
                "<p style='font-size:12pt; font-weight:bold; text-align:center;"
                " color:#1a1a1a; margin-bottom:8px;'>%1</p>"
                ).arg(m_quote.siteName.toHtmlEscaped());

    // Info table — reference, date, expiry.
    html += "<table style='border-collapse:collapse; margin-bottom:12px;'>";
    html += row("Quote Reference:", quoteRef);
    html += row("Date:",            m_quote.quoteDate);
    if (!m_quote.expiryDate.isEmpty())
        html += row("Valid Until:", m_quote.expiryDate);
    html += "</table>";

    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildSectionHtml()
//
// Used for every text section (System Offered, Basis of Design, etc.).
//
// Uses a <p> with inline style rather than <h2> for the heading.
// This is intentional — QTextDocument applies its own default size scaling
// to <h2> tags that overrides inline font-size. A <p> with inline style
// gives us precise control.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePdfGenerator::buildSectionHtml(const QString &heading,
                                            const QString &body) const
{
    QString html;

    html += QString(
                "<p style='"
                "font-size:10pt;"
                "font-weight:bold;"
                "text-decoration:underline;"
                "color:#2c3e50;"
                "border-bottom:1px solid #2c3e50;"
                "padding-bottom:2px;"
                "margin-top:14px;"
                "margin-bottom:4px;"
                "'>%1</p>"
                ).arg(heading.toHtmlEscaped());

    const QStringList lines = body.split('\n');
    for (const QString &line : lines) {
        if (line.trimmed().isEmpty())
            html += "<p style='margin:4px 0;'>&nbsp;</p>";
        else
           // qDebug("Table width will be: %d px", m_pageWidthPx);
            html += QString("<p style='font-size:9pt; margin:2px 0;'>%1</p>")
                        .arg(line.toHtmlEscaped());
    }

    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildPriceTableHtml()
//
// Full-width table with 80% description / 20% amount columns.
//
// QTextDocument ignores width:100% on tables — we use m_pageWidthPx (measured
// from the real printer) so the table fills the page exactly.
//
// Column widths are set on the header row. Qt uses the first row to determine
// column widths for the entire table, so we only need them there.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePdfGenerator::buildPriceTableHtml() const
{
    QLocale locale;
    QString sym = "$";

    QString html;

    // Section heading — <p> with inline style (same reason as buildSectionHtml).
    html += QString(
        "<p style='"
        "font-size:10pt;"
        "font-weight:bold;"
        "text-decoration:underline;"
        "color:#2c3e50;"
        "border-bottom:1px solid #2c3e50;"
        "padding-bottom:2px;"
        "margin-top:14px;"
        "margin-bottom:4px;"
        "'>Pricing</p>"
        );

    // Table at exact page width.
    // Use the HTML width attribute rather than CSS width property.
    // Qt's QTextDocument honours HTML attributes on tables more reliably
    // than CSS style properties, which it sometimes ignores.
    html += QString(
                "<table width='%1' cellspacing='0' cellpadding='0'"
                " style='border-collapse:collapse; margin-bottom:4px;'>"
                ).arg(m_pageWidthPx);

    // Header row — sets the 80/20 column widths for the whole table.
    html += "<tr style='background-color:#2c3e50; color:white;'>"
            "<td style='width:80%; padding:4px 6px;"
            " font-size:9pt; font-weight:bold;'>Description</td>"
            "<td style='width:20%; padding:4px 6px;"
            " font-size:9pt; font-weight:bold;"
            " text-align:right;'>Amount</td>"
            "</tr>";

    // Data rows.
    double subtotal = 0.0;
    bool   evenRow  = false;

    for (const PriceRow &r : m_priceRows) {
        if (r.description.trimmed().isEmpty())
            continue;

        subtotal += r.amount;
        QString bg = evenRow ? "#f5f5f5" : "#ffffff";
        evenRow    = !evenRow;

        html += QString(
                    "<tr style='background-color:%1;'>"
                    "<td style='padding:3px 6px; font-size:9pt;'>%2</td>"
                    "<td style='padding:3px 6px; font-size:9pt;"
                    " text-align:right;'>%3%4</td>"
                    "</tr>"
                    ).arg(bg,
                         r.description.toHtmlEscaped(),
                         sym,
                         locale.toString(r.amount, 'f', 2));
    }

    // Totals.
    double tax   = subtotal * (m_config.taxRate / 100.0);
    double total = subtotal + tax;

    html += QString(
                "<tr style='background-color:#eeeeee;'>"
                "<td style='padding:3px 6px; font-size:9pt; font-weight:bold;"
                " text-align:right; border-top:2px solid #2c3e50;'>"
                "Subtotal (ex %1)</td>"
                "<td style='padding:3px 6px; font-size:9pt; font-weight:bold;"
                " text-align:right; border-top:2px solid #2c3e50;'>"
                "%2%3</td>"
                "</tr>"
                ).arg(m_config.taxLabel, sym, locale.toString(subtotal, 'f', 2));

    html += QString(
                "<tr style='background-color:#eeeeee;'>"
                "<td style='padding:3px 6px; font-size:9pt; font-weight:bold;"
                " text-align:right;'>%1 (%2%)</td>"
                "<td style='padding:3px 6px; font-size:9pt; font-weight:bold;"
                " text-align:right;'>%3%4</td>"
                "</tr>"
                ).arg(m_config.taxLabel,
                     QString::number(m_config.taxRate, 'f', 1),
                     sym,
                     locale.toString(tax, 'f', 2));

    html += QString(
                "<tr style='background-color:#d5dde5;'>"
                "<td style='padding:4px 6px; font-size:9pt; font-weight:bold;"
                " text-align:right;'>Total (inc %1)</td>"
                "<td style='padding:4px 6px; font-size:9pt; font-weight:bold;"
                " text-align:right;'>%2%3</td>"
                "</tr>"
                ).arg(m_config.taxLabel, sym, locale.toString(total, 'f', 2));

    html += "</table>";
    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildSignatureHtml()
//
// Closing statement (9pt bold), optional signature image, signatory name.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePdfGenerator::buildSignatureHtml() const
{
    QString html;

    if (!m_quote.contactStatement.trimmed().isEmpty()) {
        html += QString(
                    "<p style='font-size:9pt; font-weight:bold;"
                    " margin-top:16px;'>%1</p>"
                    ).arg(m_quote.contactStatement.toHtmlEscaped());
    }

    html += "<p style='margin-top:20px;'>";

    if (!m_config.signaturePath.isEmpty()) {
        QImage sig(m_config.signaturePath);
        if (!sig.isNull()) {
            sig = sig.scaledToWidth(160, Qt::SmoothTransformation);

            QByteArray sigData;
            QBuffer    sigBuffer(&sigData);
            sigBuffer.open(QIODevice::WriteOnly);
            sig.save(&sigBuffer, "PNG");
            QString b64 = QString::fromLatin1(sigData.toBase64());

            html += QString("<img src='data:image/png;base64,%1' width='160'/><br/>")
                        .arg(b64);
        }
    }

    html += QString(
                "<span style='font-size:9pt; font-weight:bold;'>%1</span><br/>"
                "<span style='font-size:9pt; color:#555;'>%2</span>"
                ).arg(m_quote.signatoryName.toHtmlEscaped(),
                     m_config.companyName.toHtmlEscaped());

    html += "</p>";
    return html;
}