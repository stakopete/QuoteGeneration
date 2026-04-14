// ─────────────────────────────────────────────────────────────────────────────
// quotepreviewdialog.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "quotepreviewdialog.h"
#include "animatedbutton.h"
#include "stylemanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPixmap>
#include <QLocale>
#include <QResizeEvent>
#include <QRegularExpression>

void QuotePreviewDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    QVariant prop = m_logoBanner->property("originalPixmap");
    if (!prop.isNull()) {
        QPixmap original = prop.value<QPixmap>();
        if (!original.isNull()) {
            m_logoBanner->setPixmap(
                original.scaled(
                    m_logoBanner->width(),
                    m_logoBanner->height(),
                    Qt::IgnoreAspectRatio,
                    Qt::SmoothTransformation
                    )
                );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
QuotePreviewDialog::QuotePreviewDialog(const QuoteData &quote,
                                       const QList<PriceRow> &priceRows,
                                       QWidget *parent)
    : QDialog(parent)
    , m_quote(quote)
    , m_priceRows(priceRows)
{
    m_config = Database::loadConfig();
    setWindowTitle("Quote Preview — " + quote.siteName);
    resize(900, 750);
    setupUi();
    buildPreview();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void QuotePreviewDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ── Logo banner ───────────────────────────────────────────────────────────
    m_logoBanner = new QLabel();
    m_logoBanner->setFixedHeight(100);
    m_logoBanner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_logoBanner->setAlignment(Qt::AlignCenter);
    m_logoBanner->setStyleSheet("background-color: white;");

    QPixmap logo;
    bool loaded = false;

    if (!m_config.logoPath.isEmpty())
        loaded = logo.load(m_config.logoPath);

    if (!loaded)
        loaded = logo.load(
            "D:/Qt_Projects/QuoteGeneration/resources/logos/DefaultLogo.png"
            );

    if (loaded) {
        m_logoBanner->setProperty("originalPixmap", logo);
        m_logoBanner->setScaledContents(false);
        m_logoBanner->setPixmap(
            logo.scaled(m_logoBanner->width() > 0 ? m_logoBanner->width() : 900,
                        100,
                        Qt::IgnoreAspectRatio,
                        Qt::SmoothTransformation)
            );
    } else {
        m_logoBanner->setText(
            "<h2>" + m_config.companyName + "</h2>"
            );
    }

    mainLayout->addWidget(m_logoBanner);

    // ── Preview text area ─────────────────────────────────────────────────────
    // QTextEdit in read-only mode renders HTML so we can format the
    // quote professionally with headings, tables and font styles.
    m_preview = new QTextEdit();
    m_preview->setReadOnly(true);
    m_preview->setStyleSheet(
        "QTextEdit {"
        "    background-color: white;"
        "    color: #1a1a1a;"
        "    border: none;"
        "    padding: 20px;"
        "    font-family: Arial, sans-serif;"
        "    font-size: 11pt;"
        "}"
        );
    mainLayout->addWidget(m_preview);

    // ── Button bar ────────────────────────────────────────────────────────────
    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->setContentsMargins(16, 8, 16, 8);

    m_printButton = new AnimatedButton("Print / Save PDF");
    m_printButton->setFixedSize(150, 40);
    // Print will be wired up in Phase 6.
    m_printButton->setEnabled(false);
    m_printButton->setToolTip("PDF generation coming in Phase 6");

    m_closeButton = new AnimatedButton("Close");
    m_closeButton->setFixedSize(120, 40);
    connect(m_closeButton, &AnimatedButton::clicked,
            this, &QuotePreviewDialog::onClose);

    buttonRow->addWidget(m_printButton);
    buttonRow->addStretch();
    buttonRow->addWidget(m_closeButton);

    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildPreview()
//
// Assembles all quote sections into a single HTML document.
// Sections that are empty are omitted automatically.
// ─────────────────────────────────────────────────────────────────────────────
void QuotePreviewDialog::buildPreview()
{
    QString html;

    // ── Document styles ───────────────────────────────────────────────────────
    html += R"(
    <style>
        body {
            font-family: Arial, sans-serif;
            font-size: 10pt;
            color: #1a1a1a;
            line-height: 1.5;
            text-align: justify;
            margin: 0;
            padding: 0;
        }
        h1.quote-title {
            font-size: 11pt;
            color: #1a1a1a;
            text-align: center;
            margin-bottom: 2px;
            margin-top: 4px;
        }
        h1.site-name {
            font-size: 12pt;
            color: #1a1a1a;
            text-align: center;
            font-weight: bold;
            margin-top: 2px;
            margin-bottom: 4px;
        }
        h2 {
            font-size: 10pt;
            color: #2c3e50;
            border-bottom: 1px solid #2c3e50;
            padding-bottom: 2px;
            margin-top: 12px;
            margin-bottom: 4px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        h3 {
            font-size: 10pt;
            color: #2c3e50;
            margin-top: 8px;
            margin-bottom: 2px;
            font-style: italic;
        }
        p {
            margin: 3px 0;
            font-size: 10pt;
        }
        p.date {
            font-size: 9pt;
            text-align: left;
            margin-bottom: 4px;
        }
        p.draft-stamp {
            font-size: 9pt;
            text-align: center;
            color: #cc0000;
            margin: 2px 0;
        }
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 8px 0;
            font-size: 10pt;
        }
        th {
            background-color: #2c3e50;
            color: white;
            padding: 5px 8px;
            text-align: left;
            font-size: 10pt;
        }
        td {
            padding: 4px 8px;
            border-bottom: 1px solid #cccccc;
            font-size: 10pt;
        }
        td.amount {
            text-align: right;
        }
        td.total {
            font-weight: bold;
            text-align: right;
        }
        .total-row {
            font-weight: bold;
            background-color: #f5f5f5;
        }
        .grand-total {
            font-weight: bold;
            background-color: #e8eaed;
        }
        ol {
            margin: 4px 0;
            padding-left: 20px;
            font-size: 10pt;
        }
        li {
            margin: 2px 0;
        }
    </style>
    )";

    // Draft stamp — centred.
    html += "<p class='draft-stamp'><b>" +
            m_quote.status.toUpper() + "</b></p>";

    // Date — left aligned, smaller font.
    html += "<p class='date'>" + m_quote.quoteDate + "</p>";

    // ── Quote title ───────────────────────────────────────────────────────────
    html += "<p style='text-align:center; font-size:11pt; "
            "color:#1a1a1a; margin:2px 0;'>" +
            m_quote.titleText.toHtmlEscaped() + "</p>";
    html += "<p style='text-align:center; font-size:12pt; "
            "font-weight:bold; text-decoration:underline; "
            "color:#1a1a1a; margin:2px 0;'>" +
            m_quote.siteName.toHtmlEscaped() + "</p>";

    // ── System Offered ────────────────────────────────────────────────────────
    if (!m_quote.systemText.isEmpty()) {
        html += sectionHeading("System Offered");
        // Split into lines and render as an indented list
        // matching the style of other sections.
        QStringList sysLines = m_quote.systemText.split("\n",
                                                        Qt::SkipEmptyParts);
        html += "<ol>";
        for (const QString &line : sysLines) {
            html += "<li>" + line.trimmed().toHtmlEscaped() + "</li>";
        }
        html += "</ol>";
    }

    // ── Basis of Proposal ─────────────────────────────────────────────────────
    if (!m_quote.basisText.isEmpty()) {
        html += sectionHeading("Basis of Proposal");
        html += "<p>This proposal has been prepared in accordance "
                "with the following: -</p>";
        html += processTaggedText(m_quote.basisText,
                                  "Wet Fire Systems",
                                  "Dry Fire Systems");
    }

    // ── Proposed Price ────────────────────────────────────────────────────────
    if (!m_priceRows.isEmpty()) {
        html += sectionHeading("Proposed Price");
        html += buildPriceTable();
    }

    // ── Scope of Works ────────────────────────────────────────────────────────
    if (!m_quote.scopeText.isEmpty()) {
        html += sectionHeading("Scope of Works");
        html += processTaggedText(m_quote.scopeText,
                                  "Wet Fire Works",
                                  "Dry Fire Works");
    }

    // ── Exclusions ────────────────────────────────────────────────────────────
    if (!m_quote.exclusions.isEmpty()) {
        html += sectionHeading("Exclusions");
        html += processTaggedText(m_quote.exclusions,
                                  "Wet Fire Exclusions",
                                  "Dry Fire Exclusions");
    }

    // ── General Conditions ────────────────────────────────────────────────────
    if (!m_quote.generalConditions.isEmpty()) {
        html += sectionHeading("General Conditions");
        html += processTaggedText(m_quote.generalConditions,
                                  "Wet Fire Conditions",
                                  "Dry Fire Conditions");
    }

    // ── Clarifications ────────────────────────────────────────────────────────
    if (!m_quote.clarifications.isEmpty()) {
        html += sectionHeading("Clarifications");

        // Clarifications have no Wet/Dry split — render as simple list.
        QStringList lines = m_quote.clarifications.split("\n",
                                                         Qt::SkipEmptyParts);
        html += "<ol>";
        for (const QString &line : lines) {
            QString text = line.trimmed();
            // Strip number prefix if present.
            int dotPos = text.indexOf(". ");
            if (dotPos > 0 && dotPos < 4)
                text = text.mid(dotPos + 2);
            html += "<li>" + text.toHtmlEscaped() + "</li>";
        }
        html += "</ol>";
    }

    // ── Signature Block ───────────────────────────────────────────────────────
    html += "<br><br>";
    html += "<p>" + m_quote.contactStatement.toHtmlEscaped()
                        .replace("\n", "<br>") + "</p>";
    html += "<br>";
    html += "<p>Yours Sincerely,</p>";
    html += "<br>";

    // Signature image.
    if (!m_config.signaturePath.isEmpty()) {
        html += QString("<img src='%1' height='70'><br>")
        .arg(m_config.signaturePath);
    } else {
        html += "<br><br><br>";
    }

    html += "<p><b>" + m_quote.signatoryName + "</b></p>";
    html += "<p>" + m_config.companyName + "</p>";
    html += "<p>" + m_config.phone + "</p>";
    html += "<p>" + m_config.email + "</p>";

    m_preview->setHtml(html);
}

// ─────────────────────────────────────────────────────────────────────────────
// sectionHeading()
//
// Returns an HTML section heading.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePreviewDialog::sectionHeading(const QString &title) const
{
    return QString(
        "<p style='"
        "font-size:9pt;"
        "font-weight:bold;"
        "text-decoration:underline;"
        "color:#2c3e50;"
        "margin-top:12px;"
        "margin-bottom:4px;"
        "letter-spacing:1px;"
        "'>" + title.toUpper() + "</p>"
        );
}

QString QuotePreviewDialog::processTaggedText(const QString &text,
                                              const QString &wetHeading,
                                              const QString &dryHeading) const
{
    QStringList wetItems;
    QStringList dryItems;
    QStringList genItems;

    QStringList lines = text.split("\n", Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();

        // Strip the number prefix — handles any number of digits.
        // e.g. "1. [WET] text" or "10. [DRY] text"
        QRegularExpression numPrefix("^\\d+\\.\\s+");
        trimmed = trimmed.remove(numPrefix);

        // Sort by tag.
        if (trimmed.startsWith("[WET] ")) {
            wetItems.append(trimmed.mid(6));
        } else if (trimmed.startsWith("[DRY] ")) {
            dryItems.append(trimmed.mid(6));
        } else if (trimmed.startsWith("[GEN] ")) {
            genItems.append(trimmed.mid(6));
        } else {
            // No tag — treat as general.
            genItems.append(trimmed);
        }
    }

    QString html;

    if (!wetItems.isEmpty()) {
        html += "<p style='font-size:10pt; font-weight:bold; "
                "font-style:italic; color:#2c3e50; margin:6px 0 2px 0;'>"
                + wetHeading + "</p><ol>";
        for (const QString &item : wetItems)
            html += "<li>" + item.toHtmlEscaped() + "</li>";
        html += "</ol>";
    }

    if (!dryItems.isEmpty()) {
        html += "<p style='font-size:10pt; font-weight:bold; "
                "font-style:italic; color:#2c3e50; margin:6px 0 2px 0;'>"
                + dryHeading + "</p><ol>";
        for (const QString &item : dryItems)
            html += "<li>" + item.toHtmlEscaped() + "</li>";
        html += "</ol>";
    }

    if (!genItems.isEmpty()) {
        if (!wetItems.isEmpty() || !dryItems.isEmpty())
            html += "<p style='font-size:10pt; font-weight:bold; "
                    "font-style:italic; color:#2c3e50; margin:6px 0 2px 0;'>"
                    "General</p>";
        html += "<ol>";
        for (const QString &item : genItems)
            html += "<li>" + item.toHtmlEscaped() + "</li>";
        html += "</ol>";
    }

    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// buildPriceTable()
//
// Builds an HTML table for the proposed price section.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePreviewDialog::buildPriceTable() const
{
    QLocale locale;
    QString sym = "$";
    if (m_config.taxLabel == "VAT") {
        sym = qAbs(m_config.taxRate - 20.0) < 0.01 ? "£" : "€";
    }

    QString html;

    // Use a QTextDocument friendly table format.
    // Qt's rich text engine respects width attributes on td/th elements.
    html += "<table border='1' cellspacing='0' cellpadding='5' "
            "style='border-collapse:collapse; width:100%;'>";

    // Header.
    html += "<tr bgcolor='#2c3e50'>"
            "<td width='80%' style='color:white; font-weight:bold;'>"
            "Description</td>"
            "<td width='20%' align='right' "
            "style='color:white; font-weight:bold;'>"
            "Amount</td>"
            "</tr>";

    // Data rows.
    double subtotal = 0.0;
    for (const PriceRow &row : m_priceRows) {
        if (row.description.trimmed().isEmpty())
            continue;
        subtotal += row.amount;
        html += QString(
                    "<tr>"
                    "<td width='80%'>%1</td>"
                    "<td width='20%' align='right'>%2%3</td>"
                    "</tr>"
                    ).arg(row.description.toHtmlEscaped(),
                         sym,
                         locale.toString(row.amount, 'f', 2));
    }

    double tax   = subtotal * (m_config.taxRate / 100.0);
    double total = subtotal + tax;



    // Subtotal.
    html += QString(
                "<tr bgcolor='#f5f5f5'>"
                "<td width='80%' align='right' "
                "style='font-weight:bold; border-top:2px solid #2c3e50;'>"
                "Subtotal (ex %1)</td>"
                "<td width='20%' align='right' "
                "style='font-weight:bold; border-top:2px solid #2c3e50;'>"
                "%2%3</td>"
                "</tr>"
                ).arg(m_config.taxLabel, sym, locale.toString(subtotal, 'f', 2));

    // Tax.
    html += QString(
                "<tr bgcolor='#f5f5f5'>"
                "<td width='80%' align='right' style='font-weight:bold;'>"
                "%1 (%2%)</td>"
                "<td width='20%' align='right' style='font-weight:bold;'>"
                "%3%4</td>"
                "</tr>"
                ).arg(m_config.taxLabel,
                     QString::number(m_config.taxRate, 'f', 1),
                     sym,
                     locale.toString(tax, 'f', 2));

    // Total.
    html += QString(
                "<tr bgcolor='#e8eaed'>"
                "<td width='80%' align='right' "
                "style='font-weight:bold; font-size:11pt;'>"
                "Total (inc %1)</td>"
                "<td width='20%' align='right' "
                "style='font-weight:bold; font-size:11pt;'>"
                "%2%3</td>"
                "</tr>"
                ).arg(m_config.taxLabel, sym, locale.toString(total, 'f', 2));

    html += "</table>";
    return html;
}

// ─────────────────────────────────────────────────────────────────────────────
// onClose()
// ─────────────────────────────────────────────────────────────────────────────
void QuotePreviewDialog::onClose()
{
    accept();
}