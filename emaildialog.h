#ifndef EMAILDIALOG_H
#define EMAILDIALOG_H

// ─────────────────────────────────────────────────────────────────────────────
// emaildialog.h
//
// Dialog for sending the quote PDF by email.
//
// Primary method: Windows MAPI — opens the default email client with
// the PDF already attached. Works with Outlook, Thunderbird, etc.
//
// Fallback: mailto: URL — opens the email client without attachment.
// The user is instructed to attach the PDF manually.
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>
#include <QString>

class QLineEdit;
class AnimatedButton;
class QLabel;

class EmailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EmailDialog(const QString &pdfPath,
                         const QString &siteName,
                         QWidget       *parent = nullptr);

private slots:
    void onSend();

private:
    void setupUi();
    bool sendViaMapi();
    void sendViaMailto();

    // ── Controls ──────────────────────────────────────────────────────────────
    QLineEdit      *m_toAddress;
    QLineEdit      *m_subject;
    QLineEdit      *m_body;
    QLabel         *m_pdfLabel;
    AnimatedButton *m_sendButton;
    AnimatedButton *m_cancelButton;

    // ── Data ──────────────────────────────────────────────────────────────────
    QString m_pdfPath;
    QString m_siteName;
};

#endif // EMAILDIALOG_H
