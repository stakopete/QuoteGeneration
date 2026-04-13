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

private:
    void setupMenuBar();
    void setupToolBar();
    void setupLogoBanner();
    void setupTabs();
    void setupStatusBar();
    void applyStyleSheet();
    void resizeEvent(QResizeEvent *event) override;

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
};

#endif // MAINWINDOW_H