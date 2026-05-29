#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ─────────────────────────────────────────────────────────────────────────────
// mainwindow.h
//
// The main application window.
// ─────────────────────────────────────────────────────────────────────────────

#include <QMainWindow>
#include "database.h"
#include <QResizeEvent>
#include "stylemanager.h"
#include <QTimer>

// Forward declarations — we only need the full type definitions
// in the .cpp file, not here in the header.
class QTabWidget;
class QLabel;
class QToolBar;
class QAction;
class TitleSection;
class SystemSection;
class BasisSection;
class PriceSection;
class ScopeSection;
class ExclusionsSection;
class GeneralConditionsSection;
class ClarificationsSection;
class SignatureSection;
class QuoteTypeDialog;
class QuoteListDialog;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewQuote();
    void onOpenQuote();
    void onPreviewQuote();
    void onGeneratePdf();
    void onSettings();
    void onAbout();
    void onTabChanged(int index);
    void onToggleDarkMode();

    void onAutoSave();
    void onQuoteDataChanged();
    void saveCurrentQuote();
    void loadQuote(int id);
    void newQuote(const QString &quoteType = "Combined");
    void onDropdownManager();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void setupMenuBar();
    void setupToolBar();
    void setupLogoBanner();
    void setupTabs();
    void setupStatusBar();
    void applyStyleSheet();
    void resizeEvent(QResizeEvent *event) override;
    void checkExpiringQuotes();
    void openPdf(const QString &filePath);

    QTabWidget               *m_tabs;
    QLabel                   *m_logoBanner;
    QLabel                   *m_statusLabel;
    QAction                  *m_actNewQuote;
    QAction                  *m_actOpenQuote;
    QAction                  *m_actPreview;
    QAction                  *m_actGeneratePdf;
    QAction                  *m_actSettings;
    QAction                  *m_actAbout;
    AppConfig                 m_config;
    TitleSection             *m_titleSection;
    SystemSection            *m_systemSection;
    QAction                  *m_actToggleDarkMode;
    BasisSection             *m_basisSection;
    PriceSection             *m_priceSection;
    ScopeSection             *m_scopeSection;
    ExclusionsSection        *m_exclusionsSection;
    GeneralConditionsSection *m_generalSection;
    ClarificationsSection    *m_clarificationsSection;
    SignatureSection         *m_signatureSection;
    QTimer                   *m_saveDebounceTimer;
    QString                   m_lastPdfPath;

    // ── Quote management ──────────────────────────────────────────────────────
    QuoteData   m_currentQuote;     // The quote currently being edited
    QTimer     *m_autoSaveTimer;    // Fires every 2 minutes to auto-save
    bool        m_quoteModified;    // True if unsaved changes exist
    QLabel     *m_lastSavedLabel;   // Shows last saved time in status bar
};



#endif // MAINWINDOW_H