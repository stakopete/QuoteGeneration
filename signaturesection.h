#ifndef SIGNATURESECTION_H
#define SIGNATURESECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// signaturesection.h
//
// Signature Block section. The final section of the quote.
// Contains:
// - Contact statement (pre-filled from AppConfig, editable)
// - Yours Sincerely opening
// - Signature image (loaded from AppConfig)
// - Contact details below signature
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"

class QLineEdit;
class QLabel;
class QGroupBox;
class QTextEdit;
class AnimatedButton;

class SignatureSection : public QWidget
{
    Q_OBJECT

public:
    explicit SignatureSection(QWidget *parent = nullptr);

    // Returns the contact statement text.
    QString contactStatement() const;

    // Returns the signatory name.
    QString signatoryName() const;

    // Returns the signature image path.
    QString signaturePath() const;

    // Always complete — signature block always appears in the quote.
    bool isComplete() const { return true; }

    // Loads previously saved data.
    void loadData(const QString &statement, const QString &name);

signals:
    void dataChanged();

private slots:
    void onBrowseSignature();
    void onClearSignature();

private:
    void setupUi();
    void applyGroupBoxStyle(QGroupBox *group);
    void loadSignaturePreview(const QString &path);

    // ── Controls ──────────────────────────────────────────────────────────────
    QTextEdit      *m_contactStatement;
    QLineEdit      *m_signatoryName;
    QLineEdit      *m_signaturePathEdit;
    QLabel         *m_signaturePreview;
    QLabel         *m_contactDetails;
    AnimatedButton *m_browseButton;
    AnimatedButton *m_clearButton;

    // ── Data ──────────────────────────────────────────────────────────────────
    AppConfig       m_config;
};

#endif // SIGNATURESECTION_H
