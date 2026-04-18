// ─────────────────────────────────────────────────────────────────────────────
// emaildialog.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "emaildialog.h"
#include "animatedbutton.h"
#include "stylemanager.h"
#include "appsettings.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include <QUrlQuery>
#include "emaildialog.h"
#include <QDir>
#include <QGuiApplication>
#include <QClipboard>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mapi.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
EmailDialog::EmailDialog(const QString &pdfPath,
                         const QString &siteName,
                         QWidget       *parent)
    : QDialog(parent)
    , m_pdfPath(pdfPath)
    , m_siteName(siteName)
{
    setWindowTitle("Send Quote by Email");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    resize(500, 300);
    setupUi();
}

// ─────────────────────────────────────────────────────────────────────────────
// setupUi()
// ─────────────────────────────────────────────────────────────────────────────
void EmailDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

    // ── Heading ───────────────────────────────────────────────────────────────
    QLabel *heading = new QLabel("Send Quote by Email");
    heading->setStyleSheet(StyleManager::instance().headingLabelStyle());
    mainLayout->addWidget(heading);

    // ── PDF info ──────────────────────────────────────────────────────────────
    QFileInfo fi(m_pdfPath);
    m_pdfLabel = new QLabel("Attachment: " + fi.fileName());
    m_pdfLabel->setStyleSheet(
        QString("QLabel { color: %1; font-style: italic; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(m_pdfLabel);

    // ── Form fields ───────────────────────────────────────────────────────────
    QFormLayout *form = new QFormLayout();
    form->setSpacing(10);

    // To address.
    m_toAddress = new QLineEdit();
    m_toAddress->setPlaceholderText("recipient@example.com");
    m_toAddress->setStyleSheet(StyleManager::instance().lineEditStyle());
    form->addRow("To:", m_toAddress);

    // Subject — pre-filled with site name.
    m_subject = new QLineEdit();
    m_subject->setText("Quotation — " + m_siteName);
    m_subject->setStyleSheet(StyleManager::instance().lineEditStyle());
    form->addRow("Subject:", m_subject);

    // Body — short covering message.
    m_body = new QLineEdit();
    m_body->setText(
        "Please find attached our quotation for the above project."
        );
    m_body->setStyleSheet(StyleManager::instance().lineEditStyle());
    form->addRow("Message:", m_body);

    mainLayout->addLayout(form);

    // ── Note about attachment ─────────────────────────────────────────────────
    QLabel *note = new QLabel(
        "The PDF will be attached automatically where supported.\n"
        "If your email client does not support automatic attachment,\n"
        "the PDF path will be shown so you can attach it manually."
        );
    note->setWordWrap(true);
    note->setStyleSheet(
        QString("QLabel { color: %1; font-size: 10px; }")
            .arg(StyleManager::instance().labelColour())
        );
    mainLayout->addWidget(note);

    mainLayout->addStretch();

    // ── Button row ────────────────────────────────────────────────────────────
    QHBoxLayout *buttonRow = new QHBoxLayout();

    m_cancelButton = new AnimatedButton("Cancel");
    m_cancelButton->setFixedSize(100, 36);
    connect(m_cancelButton, &AnimatedButton::clicked,
            this, &QDialog::reject);
    buttonRow->addWidget(m_cancelButton);

    buttonRow->addStretch();

    m_sendButton = new AnimatedButton("Send");
    m_sendButton->setFixedSize(100, 36);
    connect(m_sendButton, &AnimatedButton::clicked,
            this, &EmailDialog::onSend);
    buttonRow->addWidget(m_sendButton);



    mainLayout->addLayout(buttonRow);
}

// ─────────────────────────────────────────────────────────────────────────────
// onSend()
// ─────────────────────────────────────────────────────────────────────────────
void EmailDialog::onSend()
{
    // Validate recipient address.
    QString to = m_toAddress->text().trimmed();
    if (to.isEmpty() || !to.contains('@')) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Invalid Address");
        msgBox.setText("Please enter a valid email address.");
        msgBox.setIcon(QMessageBox::Warning);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

#ifdef Q_OS_WIN
    if (sendViaMapi()) {
        accept();
        return;
    }
#endif

    // MAPI failed or not on Windows — show instructions for webmail users.
    QString pdfNativePath = QDir::toNativeSeparators(m_pdfPath);

    // Copy email details to clipboard so user can paste into webmail.
    QString clipText = QString(
                           "To: %1\nSubject: %2\n\n%3\n\nAttach PDF from:\n%4"
                           ).arg(to, m_subject->text().trimmed(),
                                m_body->text().trimmed(), pdfNativePath);

    QGuiApplication::clipboard()->setText(clipText);

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Open Your Email");
    msgBox.setText(
        "Your email details have been copied to the clipboard.\n\n"
        "Please open your email application or webmail and:\n"
        "1. Paste the clipboard contents\n"
        "2. Attach the PDF from:\n\n" +
        pdfNativePath + "\n\n"
                        "Click OK to close this dialog."
        );
    msgBox.setIcon(QMessageBox::Information);
    AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
    okBtn->setFixedSize(110, 40);
    msgBox.addButton(okBtn, QMessageBox::AcceptRole);
    msgBox.exec();
    accept();
}

// ─────────────────────────────────────────────────────────────────────────────
// sendViaMapi()
//
// Uses Windows Simple MAPI to open the default email client with the
// PDF already attached. Returns true if successful.
//
// Simple MAPI is available on all Windows versions and works with
// Outlook, Thunderbird, Windows Mail and most other email clients.
// ─────────────────────────────────────────────────────────────────────────────
bool EmailDialog::sendViaMapi()
{
#ifdef Q_OS_WIN
    // Load MAPI dynamically — avoids link errors if mapi32.dll is missing.
    HMODULE mapiLib = LoadLibraryA("mapi32.dll");
    if (!mapiLib)
        return false;

    // Get the MAPISendMail function pointer.
    typedef ULONG (PASCAL *LPMAPISENDMAIL)(
        LHANDLE, ULONG_PTR, lpMapiMessage, FLAGS, ULONG
        );

    LPMAPISENDMAIL MAPISendMail =
        (LPMAPISENDMAIL)GetProcAddress(mapiLib, "MAPISendMail");

    if (!MAPISendMail) {
        FreeLibrary(mapiLib);
        return false;
    }

    // Convert Qt strings to C-style for MAPI.
    QByteArray toBytes      = m_toAddress->text().trimmed().toLocal8Bit();
    QByteArray subjectBytes = m_subject->text().trimmed().toLocal8Bit();
    QByteArray bodyBytes    = m_body->text().trimmed().toLocal8Bit();
    QByteArray pathBytes    = QDir::toNativeSeparators(m_pdfPath).toLocal8Bit();
    QByteArray fileNameBytes = QFileInfo(m_pdfPath).fileName().toLocal8Bit();

    // Set up the recipient.
    MapiRecipDesc recipient = {};
    recipient.ulRecipClass  = MAPI_TO;
    recipient.lpszName      = toBytes.data();
    recipient.lpszAddress   = toBytes.data();

    // Set up the attachment.
    MapiFileDesc attachment = {};
    attachment.nPosition    = (ULONG)-1;  // No position in body text.
    attachment.lpszPathName = pathBytes.data();
    attachment.lpszFileName = fileNameBytes.data();

    // Set up the message.
    MapiMessage message   = {};
    message.lpszSubject   = subjectBytes.data();
    message.lpszNoteText  = bodyBytes.data();
    message.nRecipCount   = 1;
    message.lpRecips      = &recipient;
    message.nFileCount    = 1;
    message.lpFiles       = &attachment;

    // MAPI_DIALOG shows the email client compose window so the user
    // can review and send — we never send silently without user approval.
    ULONG result = MAPISendMail(0, 0, &message, MAPI_DIALOG, 0);

    FreeLibrary(mapiLib);

    // SUCCESS_SUCCESS = 0, USER_ABORT = 1 (user closed window — not an error).
    return (result == SUCCESS_SUCCESS || result == MAPI_E_USER_ABORT);
#else
    return false;
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// sendViaMailto()
//
// Fallback — opens the default email client via a mailto: URL.
// Attachment is not supported but recipient, subject and body are pre-filled.
// The user is shown the PDF path so they can attach it manually.
// ─────────────────────────────────────────────────────────────────────────────
void EmailDialog::sendViaMailto()
{
    QString body = m_body->text().trimmed()
    + "\n\nPlease attach the PDF manually from:\n"
        + QDir::toNativeSeparators(m_pdfPath);

    QUrl url("mailto:" + m_toAddress->text().trimmed());
    QUrlQuery query;
    query.addQueryItem("subject", m_subject->text().trimmed());
    query.addQueryItem("body",    body);
    url.setQuery(query);

    QDesktopServices::openUrl(url);
}