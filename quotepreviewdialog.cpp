// ─────────────────────────────────────────────────────────────────────────────
// quotepreviewdialog.cpp
//
// Screen preview of the quote. The HTML built here deliberately mirrors the
// PDF generator so what you see on screen matches what is printed.
//
// Key rules carried over from the PDF generator:
//   1. No <h2> tags — QTextDocument applies its own size multiplier to <h2>
//      that overrides any font-size you specify. We use <p> with inline styles.
//   2. No width:100% on tables — QTextDocument ignores it. We use width:100%
//      via the HTML width attribute instead (width='100%' on the <table> tag).
//   3. All font sizes are inline style attributes — <style> block rules are
//      not reliably applied to block elements by QTextDocument.
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
#include <QDate>
#include <QDir>
#include "quotepdfgenerator.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include "emaildialog.h"

#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include "appsettings.h"

// ─────────────────────────────────────────────────────────────────────────────
// resizeEvent()
//
// Rescales the logo banner whenever the dialog is resized so it always
// fills the full width of the window.
// ─────────────────────────────────────────────────────────────────────────────
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
//
// Builds the dialog layout:
//   - Logo banner at the top (full width QLabel)
//   - QTextEdit in read-only mode for the HTML preview
//   - Button bar at the bottom
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
        // Store the original so resizeEvent() can rescale it cleanly.
        m_logoBanner->setProperty("originalPixmap", logo);
        m_logoBanner->setScaledContents(false);
        m_logoBanner->setPixmap(
            logo.scaled(m_logoBanner->width() > 0 ? m_logoBanner->width() : 900,
                        100,
                        Qt::IgnoreAspectRatio,
                        Qt::SmoothTransformation)
            );
    } else {
        m_logoBanner->setText("<b>" + m_config.companyName + "</b>");
    }

    mainLayout->addWidget(m_logoBanner);

    // ── Preview text area ─────────────────────────────────────────────────────
    // Read-only QTextEdit renders HTML — same engine as QTextDocument in the
    // PDF generator, so font sizes and styles behave identically.
    m_preview = new QTextEdit();
    m_preview->setReadOnly(true);
    m_preview->setStyleSheet(
        "QTextEdit {"
        "    background-color: white;"
        "    color: #1a1a1a;"
        "    border: none;"
        "    padding: 20px;"
        "    font-family: 'Times New Roman', Times, serif;"
        "    font-size: 9pt;"
        "}"
        );
    mainLayout->addWidget(m_preview);

    // ── Button bar ────────────────────────────────────────────────────────────
    QHBoxLayout *buttonRow = new QHBoxLayout();
    buttonRow->setContentsMargins(16, 8, 16, 8);

    m_printButton = new AnimatedButton("Save as PDF");

    m_printButton->setFixedSize(120, 40);
    connect(m_printButton, &AnimatedButton::clicked,
            this, &QuotePreviewDialog::onGeneratePdf);

    m_closeButton = new AnimatedButton("Close");
    m_closeButton->setFixedSize(120, 40);
    connect(m_closeButton, &AnimatedButton::clicked,
            this, &QuotePreviewDialog::onClose);

    buttonRow->addWidget(m_printButton);
    buttonRow->addStretch();
    buttonRow->addWidget(m_closeButton);

    m_viewPdfButton = new AnimatedButton("View PDF");
    m_viewPdfButton->setFixedSize(110, 40);
    m_viewPdfButton->setEnabled(false);  // Disabled until PDF is generated.
    connect(m_viewPdfButton, &AnimatedButton::clicked,
            this, &QuotePreviewDialog::onViewPdf);
    buttonRow->addWidget(m_viewPdfButton);
    m_emailButton = new AnimatedButton("Email Quote");
    m_emailButton->setFixedSize(120, 40);
    m_emailButton->setEnabled(false);
    connect(m_emailButton, &AnimatedButton::clicked,
            this, &QuotePreviewDialog::onEmailQuote);
    buttonRow->addWidget(m_emailButton);

    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// buildPreview()
//
// Builds the complete HTML for the screen preview, mirroring the PDF layout.
// ─────────────────────────────────────────────────────────────────────────────
void QuotePreviewDialog::buildPreview()
{
    QString html;

    // Minimal <style> block — body defaults only.
    // All heading and element sizes use inline styles (see file header comment).
    html += R"(
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
    ol {
        margin: 4px 0;
        padding-left: 20px;
        font-size: 9pt;
    }
    li { margin: 2px 0; }
</style>
)";

    // ── Status stamp ──────────────────────────────────────────────────────────
    html += "<p style='font-size:9pt; text-align:center; color:#cc0000;"
            " margin:2px 0;'><b>" + m_quote.status.toUpper() + "</b></p>";


    // ── Quote Title — 11pt, centred, not bold ─────────────────────────────────
    if (!m_quote.titleText.trimmed().isEmpty()) {
        html += "<p style='font-size:11pt; text-align:center;"
                " color:#1a1a1a; margin:2px 0;'>"
                + m_quote.titleText.toHtmlEscaped() + "</p>";
    }

    // Double underline achieved by stacking two border-bottom lines
    // using a nested table — Qt's HTML renderer does not support
    // text-decoration:underline double directly.
    html += "<p style='font-size:12pt; font-weight:bold; text-align:center;"
            " color:#1a1a1a; margin:2px 0;'>"
            "<u>" + m_quote.siteName.toHtmlEscaped() + "</u></p>";

    // ── Quote Reference / Date table ──────────────────────────────────────────
    // Quote Type removed. Reference format YYMMnnn e.g. 2603001.
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

    // Narrow info table — width:auto so it only takes as much space as needed.
    html += "<table style='border-collapse:collapse; margin-top:8px;"
            " margin-bottom:12px; width:auto;'>";
    html += QString("<tr>"
                    "<td style='font-size:9pt; font-weight:bold;"
                    " padding:2px 6px 2px 0; width:25%;'>Quote Reference:</td>"
                    "<td style='font-size:9pt; padding:2px 0;'>%1</td>"
                    "</tr>").arg(quoteRef);
    html += QString("<tr>"
                    "<td style='font-size:9pt; font-weight:bold;"
                    " padding:2px 6px 2px 0;'>Date:</td>"
                    "<td style='font-size:9pt; padding:2px 0;'>%1</td>"
                    "</tr>").arg(m_quote.quoteDate.toHtmlEscaped());
    if (!m_quote.expiryDate.isEmpty()) {
        html += QString("<tr>"
                        "<td style='font-size:9pt; font-weight:bold;"
                        " padding:2px 6px 2px 0;'>Valid Until:</td>"
                        "<td style='font-size:9pt; padding:2px 0;'>%1</td>"
                        "</tr>").arg(m_quote.expiryDate.toHtmlEscaped());
    }
    html += "</table>";

    // ── System Offered ────────────────────────────────────────────────────────
    if (!m_quote.systemText.isEmpty()) {
        html += sectionHeading("System Offered");
        QStringList sysLines = m_quote.systemText.split("\n", Qt::SkipEmptyParts);
        html += "<ol>";
        for (const QString &line : sysLines)
            html += "<li>" + line.trimmed().toHtmlEscaped() + "</li>";
        html += "</ol>";
    }

    // ── Basis of Proposal ─────────────────────────────────────────────────────
    if (!m_quote.basisText.isEmpty()) {
        html += sectionHeading("Basis of Proposal");
        html += "<p style='font-size:9pt;'>This proposal has been prepared "
                "in accordance with the following: -</p>";
        // Basis section uses a simple numbered list regardless of quote type.
        QStringList basisLines = m_quote.basisText.split("\n", Qt::SkipEmptyParts);
        html += "<ol>";
        for (const QString &line : basisLines) {
            QString clean = line.trimmed();
            QRegularExpression numPrefix("^\\d+\\.\\s+");
            clean.remove(numPrefix);
            clean.replace("[WET] ", "").replace("[DRY] ", "").replace("[GEN] ", "");
            if (!clean.isEmpty())
                html += "<li>" + clean.toHtmlEscaped() + "</li>";
        }
        html += "</ol>";
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
                                  "Wet Fire",
                                  "Dry Fire");
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
        QStringList lines = m_quote.clarifications.split("\n", Qt::SkipEmptyParts);
        html += "<ol>";
        for (const QString &line : lines) {
            QString text = line.trimmed();
            int dotPos = text.indexOf(". ");
            if (dotPos > 0 && dotPos < 4)
                text = text.mid(dotPos + 2);
            html += "<li>" + text.toHtmlEscaped() + "</li>";
        }
        html += "</ol>";
    }

    // ── Signature block ───────────────────────────────────────────────────────
    // ── Signature block ───────────────────────────────────────────────────────
    html += "<p style='margin-top:16px;'></p>";
    if (!m_quote.contactStatement.trimmed().isEmpty()) {
        html += "<p style='font-size:9pt; font-weight:bold;'>"
                + m_quote.contactStatement.toHtmlEscaped()
                      .replace("\n", "<br>")
                + "</p>";
    }
    html += "<p style='font-size:9pt; margin-top:8px;'>Yours Sincerely,</p>";

    // Signature image or blank space for manual signature.
    if (!m_config.signaturePath.isEmpty()) {
        html += QString("<p style='margin-top:8px;'>"
                        "<img src='%1' height='70'></p>")
                    .arg(m_config.signaturePath);
    } else {
        // No signature — leave enough space for a physical signature.
        html += "<p style='margin-top:60px;'>&nbsp;</p>";
    }

    // Signatory name, company name, contact name and phone.
    html += "<p style='font-size:9pt; margin-top:4px;'><b>"
            + m_quote.signatoryName.toHtmlEscaped() + "</b><br>"
            + m_config.companyName.toHtmlEscaped() + "<br>"
            + m_config.contactName.toHtmlEscaped() + "<br>"
            + m_config.phone.toHtmlEscaped()
            + "</p>";

    m_preview->setHtml(html);
}
QString QuotePreviewDialog::sectionHeading(const QString &title) const
{
    return QString(
               "<p style='"
               "font-size:10pt;"
               "font-weight:bold;"
               "text-decoration:underline;"
               "color:#2c3e50;"
               "margin-top:14px;"
               "margin-bottom:4px;"
               "'>%1</p>"
               ).arg(title.toHtmlEscaped());
}

// ─────────────────────────────────────────────────────────────────────────────
// processTaggedText()
//
// Splits text into [WET], [DRY] and [GEN] tagged items and renders each
// group under its own sub-heading as a numbered list.
// ─────────────────────────────────────────────────────────────────────────────
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
        QRegularExpression numPrefix("^\\d+\\.\\s+");
        trimmed = trimmed.remove(numPrefix);

        if (trimmed.startsWith("[WET] "))
            wetItems.append(trimmed.mid(6));
        else if (trimmed.startsWith("[DRY] "))
            dryItems.append(trimmed.mid(6));
        else if (trimmed.startsWith("[GEN] "))
            genItems.append(trimmed.mid(6));
        else
            genItems.append(trimmed);
    }

    // For combined quotes show wet and dry subheadings.
    // For single discipline quotes just show a numbered list.
    bool isCombined = (m_quote.quoteType == "Combined");

    QString html;

    if (isCombined) {
        // Wet Fire subheading and items.
        if (!wetItems.isEmpty()) {
            html += "<p style='font-size:10pt; font-weight:bold;"
                    " font-style:italic; color:#2c3e50;"
                    " margin:6px 0 2px 0;'>" + wetHeading + "</p>";
            html += "<ol>";
            for (const QString &item : wetItems)
                html += "<li>" + item.toHtmlEscaped() + "</li>";
            html += "</ol>";
        }

        // Dry Fire subheading and items.
        if (!dryItems.isEmpty()) {
            html += "<p style='font-size:10pt; font-weight:bold;"
                    " font-style:italic; color:#2c3e50;"
                    " margin:6px 0 2px 0;'>" + dryHeading + "</p>";
            html += "<ol>";
            for (const QString &item : dryItems)
                html += "<li>" + item.toHtmlEscaped() + "</li>";
            html += "</ol>";
        }
    } else {
        // Single discipline or general — no subheadings, just a numbered list.
        QStringList allItems;
        allItems << wetItems << dryItems << genItems;
        if (!allItems.isEmpty()) {
            html += "<ol>";
            for (const QString &item : allItems)
                html += "<li>" + item.toHtmlEscaped() + "</li>";
            html += "</ol>";
        }
    }

    // General items for combined quotes.
    if (isCombined && !genItems.isEmpty()) {
        html += "<p style='font-size:10pt; font-weight:bold;"
                " font-style:italic; color:#2c3e50;"
                " margin:6px 0 2px 0;'>General</p>";
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
// Full-width price table with 80/20 column split.
//
// The screen preview uses width='100%' as an HTML attribute (not a CSS
// property) on the <table> tag. QTextDocument honours HTML attributes on
// tables more reliably than CSS style properties.
// ─────────────────────────────────────────────────────────────────────────────
QString QuotePreviewDialog::buildPriceTable() const
{
    QLocale locale;
    QString sym = "$";
    if (m_config.taxLabel == "VAT")
        sym = qAbs(m_config.taxRate - 20.0) < 0.01 ? "£" : "€";

    QString html;

    // width='100%' as HTML attribute — more reliable than CSS in QTextDocument.
    html += "<table width='100%' border='1' cellspacing='0' cellpadding='4'"
            " style='border-collapse:collapse; margin-bottom:4px;'>";

    // Header row — column widths set here apply to the whole table.
    html += "<tr bgcolor='#2c3e50'>"
            "<td width='80%' style='color:white; font-size:9pt;"
            " font-weight:bold; padding:4px 6px;'>Description</td>"
            "<td width='20%' align='right' style='color:white; font-size:9pt;"
            " font-weight:bold; padding:4px 6px;'>Amount</td>"
            "</tr>";

    double subtotal = 0.0;
    bool   evenRow  = false;

    for (const PriceRow &row : m_priceRows) {
        if (row.description.trimmed().isEmpty())
            continue;
        subtotal += row.amount;
        QString bg = evenRow ? "#f5f5f5" : "#ffffff";
        evenRow    = !evenRow;
        html += QString(
                    "<tr bgcolor='%1'>"
                    "<td width='80%' style='font-size:9pt;"
                    " padding:3px 6px;'>%2</td>"
                    "<td width='20%' align='right' style='font-size:9pt;"
                    " padding:3px 6px;'>%3%4</td>"
                    "</tr>"
                    ).arg(bg,
                         row.description.toHtmlEscaped(),
                         sym,
                         locale.toString(row.amount, 'f', 2));
    }

    double tax   = subtotal * (m_config.taxRate / 100.0);
    double total = subtotal + tax;

    html += QString(
                "<tr bgcolor='#eeeeee'>"
                "<td width='80%' align='right' style='font-size:9pt;"
                " font-weight:bold; border-top:2px solid #2c3e50;"
                " padding:3px 6px;'>Subtotal (ex %1)</td>"
                "<td width='20%' align='right' style='font-size:9pt;"
                " font-weight:bold; border-top:2px solid #2c3e50;"
                " padding:3px 6px;'>%2%3</td>"
                "</tr>"
                ).arg(m_config.taxLabel, sym, locale.toString(subtotal, 'f', 2));

    html += QString(
                "<tr bgcolor='#eeeeee'>"
                "<td width='80%' align='right' style='font-size:9pt;"
                " font-weight:bold; padding:3px 6px;'>%1 (%2%)</td>"
                "<td width='20%' align='right' style='font-size:9pt;"
                " font-weight:bold; padding:3px 6px;'>%3%4</td>"
                "</tr>"
                ).arg(m_config.taxLabel,
                     QString::number(m_config.taxRate, 'f', 1),
                     sym,
                     locale.toString(tax, 'f', 2));

    html += QString(
                "<tr bgcolor='#d5dde5'>"
                "<td width='80%' align='right' style='font-size:9pt;"
                " font-weight:bold; padding:4px 6px;'>Total (inc %1)</td>"
                "<td width='20%' align='right' style='font-size:9pt;"
                " font-weight:bold; padding:4px 6px;'>%2%3</td>"
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

// ─────────────────────────────────────────────────────────────────────────────
// onGeneratePdf()
// ─────────────────────────────────────────────────────────────────────────────
void QuotePreviewDialog::onGeneratePdf()
{
    QString safeName = m_quote.siteName;
    safeName.replace(QRegularExpression(R"([\\/:*?"<>|])"), "_");
    if (safeName.trimmed().isEmpty())
        safeName = "Quote";

    // Reference string in YYMMnnn format — matches the PDF.
    QString ref;
    if (m_quote.id > 0) {
        QDate   today = QDate::currentDate();
        QString yy    = today.toString("yy");
        QString mm    = today.toString("MM");
        QString seq   = QString("%1").arg(m_quote.id, 3, 10, QChar('0'));
        ref = yy + mm + seq;
    } else {
        ref = "Draft";
    }

    QString defaultName = QString("%1_%2.pdf").arg(ref, safeName);

    QString quotesPath = "D:/Quotes";
    QDir quotesDir(quotesPath);
    if (!quotesDir.exists())
        quotesDir.mkpath(quotesPath);

    QString defaultPath = quotesPath + "/" + defaultName;

    QString filePath = QFileDialog::getSaveFileName(
        this,
        "Save Quote as PDF",
        defaultPath,
        "PDF Files (*.pdf)");

    if (filePath.isEmpty())
        return;

    QuotePdfGenerator generator(m_quote, m_priceRows, m_config);
    bool ok = generator.generate(filePath);

    if (ok) {
        QMessageBox::information(
            this,
            "PDF Saved",
            QString("Quote saved successfully:\n%1").arg(filePath));
    } else {
        QMessageBox::critical(
            this,
            "PDF Error",
            QString("The PDF could not be saved to:\n%1\n\n"
                    "Check that the folder exists and you have "
                    "permission to write there.").arg(filePath));
    }
    // Store the path and enable the View PDF and Email buttons.
    m_lastPdfPath = filePath;
    m_viewPdfButton->setEnabled(true);
    m_emailButton->setEnabled(true);
}



void QuotePreviewDialog::onViewPdf()
{
    if (m_lastPdfPath.isEmpty())
        return;

    QString sumatraPath = AppSettings::instance().sumatraPdfPath();

    if (!sumatraPath.isEmpty() && QFileInfo::exists(sumatraPath)) {
        QProcess::startDetached(sumatraPath, {m_lastPdfPath});
    } else {
        QDesktopServices::openUrl(QUrl::fromLocalFile(m_lastPdfPath));
    }
}

void QuotePreviewDialog::onEmailQuote()
{
    if (m_lastPdfPath.isEmpty())
        return;

    EmailDialog dlg(m_lastPdfPath, m_quote.siteName, this);
    dlg.exec();
}