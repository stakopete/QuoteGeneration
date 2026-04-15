// ─────────────────────────────────────────────────────────────────────────────
// mainwindow.cpp
//
// Implementation of the main application window.
// ─────────────────────────────────────────────────────────────────────────────

#include "mainwindow.h"
#include "configdialog.h"
#include "titlesection.h"
#include "systemsection.h"
#include "stylemanager.h"

#include <QTabWidget>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QVBoxLayout>
#include <QWidget>
#include <QPixmap>
#include <QMessageBox>
#include <QApplication>
#include <QPalette>
#include <QTabBar>
#include "basissection.h"
#include "pricesection.h"
#include "scopesection.h"
#include "exclusionssection.h"
#include "generalconditionssection.h"
#include "clarificationssection.h"
#include "signaturesection.h"
#include <QTimer>
#include <QCloseEvent>
#include <QDate>
#include "quotepreviewdialog.h"
#include "quotetypedialog.h"
#include "dropdownmanagerdialog.h"


// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Load config so we have company name and logo path available.
    m_config = Database::loadConfig();

    // Set the window title to include the company name.
    setWindowTitle("Quote Generation — " + m_config.companyName);

    // Set initial size but allow the user to resize freely.
    setMinimumSize(1024, 700);
    resize(1024, 768);

    // Set background colour directly rather than via stylesheet
    // to avoid interfering with the native window frame on Windows.
    // Silver grey background — more professional than white.
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor("#c0c0c0"));
    pal.setColor(QPalette::Base, QColor("#c0c0c0"));
    setPalette(pal);

    // Build the UI in the correct order — menu first, then toolbar,
    // then the central content area, then status bar at the bottom.
    setupMenuBar();
    setupToolBar();
    setupLogoBanner();
    setupTabs();
    setupStatusBar();
    applyStyleSheet();

    // ── Quote management setup ────────────────────────────────────────────────
    m_quoteModified = false;
    m_currentQuote  = QuoteData();

    // Auto-save timer fires every 2 minutes.
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(2 * 60 * 1000);    // 2 minutes in ms
    connect(m_autoSaveTimer, &QTimer::timeout,
            this, &MainWindow::onAutoSave);
    m_autoSaveTimer->start();

    // Debounce timer — saves 3 seconds after the user stops making changes.
    // This prevents saving on every single keystroke.
    m_saveDebounceTimer = new QTimer(this);
    m_saveDebounceTimer->setInterval(3000);  // 3 seconds
    m_saveDebounceTimer->setSingleShot(true); // Only fires once then stops.
    connect(m_saveDebounceTimer, &QTimer::timeout,
            this, &MainWindow::saveCurrentQuote);

    // Connect all section dataChanged signals to our handler.
    connect(m_titleSection,          &TitleSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_systemSection,         &SystemSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_basisSection,          &BasisSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_priceSection,          &PriceSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_scopeSection,          &ScopeSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_exclusionsSection,     &ExclusionsSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_generalSection,        &GeneralConditionsSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_clarificationsSection, &ClarificationsSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);
    connect(m_signatureSection,      &SignatureSection::dataChanged,
            this, &MainWindow::onQuoteDataChanged);

    // Load the last open quote if one exists.
    int lastId = Database::lastQuoteId();
    if (lastId > 0)
        loadQuote(lastId);
    else
        newQuote();

}

MainWindow::~MainWindow()
{
    // Qt automatically deletes child widgets when the parent is destroyed.
    // We don't need to manually delete anything here.
}

// ─────────────────────────────────────────────────────────────────────────────
// setupMenuBar()
//
// Creates the application menu bar with File, Quote and Help menus.
// Menu actions are connected to the same slots as the toolbar buttons
// so both ways of doing something call the same code.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupMenuBar()
{
    // ── File menu ─────────────────────────────────────────────────────────────
    QMenu *fileMenu = menuBar()->addMenu("&File");

    m_actNewQuote = fileMenu->addAction("&New Quote");
    connect(m_actNewQuote, &QAction::triggered, this, &MainWindow::onNewQuote);

    m_actOpenQuote = fileMenu->addAction("&Open Quote");
    connect(m_actOpenQuote, &QAction::triggered, this, &MainWindow::onOpenQuote);

    fileMenu->addSeparator();

    m_actGeneratePdf = fileMenu->addAction("&Generate PDF");
    connect(m_actGeneratePdf, &QAction::triggered,
            this, &MainWindow::onGeneratePdf);

    fileMenu->addSeparator();

    QAction *actExit = fileMenu->addAction("E&xit");
    connect(actExit, &QAction::triggered, this, &QMainWindow::close);

    // ── Quote menu ────────────────────────────────────────────────────────────
    QMenu *quoteMenu = menuBar()->addMenu("&Quote");

    m_actPreview = quoteMenu->addAction("&Preview Quote");
    connect(m_actPreview, &QAction::triggered,
            this, &MainWindow::onPreviewQuote);

    // ── Tools menu ────────────────────────────────────────────────────────────
    QMenu *toolsMenu = menuBar()->addMenu("&Tools");
    m_actSettings = toolsMenu->addAction("&Settings");
    connect(m_actSettings, &QAction::triggered, this, &MainWindow::onSettings);

    m_actToggleDarkMode = toolsMenu->addAction("Toggle &Dark Mode");
    m_actToggleDarkMode->setCheckable(true);
    m_actToggleDarkMode->setChecked(StyleManager::instance().isDarkMode());
    connect(m_actToggleDarkMode, &QAction::triggered,
            this, &MainWindow::onToggleDarkMode);

    // ── Help menu ─────────────────────────────────────────────────────────────
    QMenu *helpMenu = menuBar()->addMenu("&Help");

    m_actAbout = helpMenu->addAction("&About");
    connect(m_actAbout, &QAction::triggered, this, &MainWindow::onAbout);
}

// ─────────────────────────────────────────────────────────────────────────────
// setupToolBar()
//
// Creates a toolbar with the most commonly used actions.
// The toolbar sits below the menu bar and provides quick access
// to the main functions without navigating menus.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupToolBar()
{
    QToolBar *toolbar = addToolBar("Main Toolbar");

    // Prevent the user from moving or floating the toolbar.
    toolbar->setMovable(false);
    toolbar->setFloatable(false);

    // Set a comfortable icon/button size.
    toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar->setIconSize(QSize(32, 32));

    // Add actions to the toolbar.
    // At this stage we don't have icon image files so we use text only.
    // Icons will be added in the polish phase.
    toolbar->addAction(m_actNewQuote);
    toolbar->addAction(m_actOpenQuote);
    toolbar->addSeparator();
    toolbar->addAction(m_actPreview);
    toolbar->addAction(m_actGeneratePdf);
    toolbar->addSeparator();
    toolbar->addAction(m_actSettings);
    toolbar->addAction(m_actAbout);
    toolbar->addAction(m_actToggleDarkMode);
}

// ─────────────────────────────────────────────────────────────────────────────
// setupLogoBanner()
//
// Creates the logo banner displayed at the top of the content area.
// If the user has configured a logo path we use that image.
// Otherwise we use the default logo from the resources folder.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupLogoBanner()
{
    m_logoBanner = new QLabel();
    m_logoBanner->setAlignment(Qt::AlignCenter);
    m_logoBanner->setFixedHeight(100);
    m_logoBanner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_logoBanner->setStyleSheet("background-color: white;");

    // Store the original pixmap as a property so we can rescale it
    // when the window is resized. We use setScaledContents here but
    // control the aspect ratio through the pixmap itself.
    QPixmap logo;
    bool loaded = false;

    if (!m_config.logoPath.isEmpty())
        loaded = logo.load(m_config.logoPath);

    if (!loaded)
        loaded = logo.load(
            "D:/Qt_Projects/QuoteGeneration/resources/logos/DefaultLogo.png"
            );

    if (loaded) {
        // Store the original full size pixmap on the label as a property.
        // We will use this in resizeEvent to rescale correctly.
        m_logoBanner->setProperty("originalPixmap", logo);
        m_logoBanner->setScaledContents(true);
        m_logoBanner->setPixmap(logo);
    } else {
        m_logoBanner->setText("<h2>" + m_config.companyName + "</h2>");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// setupTabs()
//
// Creates the central tab widget with one tab per quote section.
// At this stage the tabs contain placeholder text — we will replace
// them with real section widgets in Phase 4.
//
// The tabs are in the order they appear in the final quote document.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupTabs()
{
    m_tabs = new QTabWidget();
    m_tabs->setTabPosition(QTabWidget::North);   // Tabs along the top
    m_tabs->setDocumentMode(true);               // Cleaner look on Windows

    // Expand tabs to fill the full width of the tab bar.
    // This eliminates the dark empty space after the last tab.
    m_tabs->tabBar()->setExpanding(true);

    // Connect the tab changed signal so we know when the user switches tabs.
    connect(m_tabs, &QTabWidget::currentChanged,
            this, &MainWindow::onTabChanged);

    // ── Placeholder tabs ──────────────────────────────────────────────────────
    // Each tab gets a simple label for now.
    // Phase 4 will replace these with real section widgets.
    auto makePlaceholder = [](const QString &text) -> QWidget* {
        QWidget *w = new QWidget();
        QVBoxLayout *l = new QVBoxLayout(w);
        QLabel *lbl = new QLabel(text);
        lbl->setAlignment(Qt::AlignCenter);
        lbl->setStyleSheet("color: grey; font-size: 14px;");
        l->addWidget(lbl);
        return w;
    };

    m_titleSection = new TitleSection();
    m_tabs->addTab(m_titleSection, "Title");
    m_systemSection = new SystemSection();
    m_tabs->addTab(m_systemSection, "System");
    m_basisSection = new BasisSection();
    m_tabs->addTab(m_basisSection, "Basis");
    m_priceSection = new PriceSection();
    m_tabs->addTab(m_priceSection, "Price");
    m_scopeSection = new ScopeSection();
    m_tabs->addTab(m_scopeSection, "Scope");
    m_exclusionsSection = new ExclusionsSection();
    m_tabs->addTab(m_exclusionsSection, "Exclusions");
    m_generalSection = new GeneralConditionsSection();
    m_tabs->addTab(m_generalSection, "General");
    m_clarificationsSection = new ClarificationsSection();
    m_tabs->addTab(m_clarificationsSection, "Clarifications");
    m_signatureSection = new SignatureSection();
    m_tabs->addTab(m_signatureSection, "Signature");

    // ── Central widget ────────────────────────────────────────────────────────
    // QMainWindow requires a central widget. We wrap the logo banner
    // and the tabs together in a vertical layout inside a container widget.
    QWidget *central = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_tabs);
    setCentralWidget(central);
    // QWidget *central = new QWidget();
    // QVBoxLayout *layout = new QVBoxLayout(central);
    // layout->setContentsMargins(0, 0, 0, 0);
    // layout->setSpacing(0);

    // layout->addWidget(m_logoBanner);
    // layout->addWidget(m_tabs);

    // setCentralWidget(central);
}

// ─────────────────────────────────────────────────────────────────────────────
// setupStatusBar()
//
// The status bar runs along the bottom of the window.
// It shows the current quote status and last saved time.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("  Ready");
    m_statusLabel->setStyleSheet("color: white;");
    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->setSizeGripEnabled(true);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyStyleSheet()
//
// Applies a consistent visual style to the main window.
// We use Qt Style Sheets (QSS) which work similarly to CSS.
// This gives us the professional look described in the specification.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::applyStyleSheet()
{
    StyleManager &sm = StyleManager::instance();

    // ── Menu bar ──────────────────────────────────────────────────────────────
    menuBar()->setStyleSheet(QString(R"(
        QMenuBar {
            background-color: %1;
            color: white;
            padding: 2px;
        }
        QMenuBar::item {
            background-color: transparent;
            color: white;
            padding: 4px 8px;
        }
        QMenuBar::item:selected {
            background-color: %2;
        }
        QMenu {
            background-color: %1;
            color: white;
            border: 1px solid %3;
        }
        QMenu::item {
            color: white;
            padding: 4px 20px;
        }
        QMenu::item:selected {
            background-color: %2;
        }
        QMenu::item:checked {
            color: #7fc8f8;
        }
    )").arg(sm.menuBackground(),
                                      sm.selectionBackground(),
                                      sm.borderColour()));

    // ── Toolbar ───────────────────────────────────────────────────────────────
    QToolBar *tb = findChild<QToolBar*>();
    if (tb) {
        tb->setStyleSheet(QString(R"(
            QToolBar {
                background-color: %1;
                border: none;
                padding: 4px;
                spacing: 4px;
            }
            QToolButton {
                color: white;
                background-color: transparent;
                border: 1px solid transparent;
                border-radius: 4px;
                padding: 4px 8px;
                min-width: 60px;
            }
            QToolButton:hover {
                background-color: %2;
                border: 1px solid %3;
            }
            QToolButton:pressed {
                background-color: %4;
            }
            QToolButton:checked {
                background-color: %2;
                border: 1px solid %3;
            }
        )").arg(sm.toolbarBackground(),
                                   sm.selectionBackground(),
                                   sm.borderColour(),
                                   sm.menuBackground()));
    }

    // ── Tab widget ────────────────────────────────────────────────────────────
    m_tabs->setStyleSheet(QString(R"(
        QTabWidget::pane {
            border: 1px solid %1;
            background-color: %2;
            top: -1px;
        }
        QTabWidget::tab-bar {
            alignment: left;
        }
        QTabBar {
            background-color: %3;
        }
        QTabBar::tab {
            background-color: %4;
            color: %5;
            padding: 6px 0px;
            width: 100px;
            border: 1px solid %1;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
            margin-right: 2px;
            text-align: center;
        }
        QTabBar::tab:selected {
            background-color: %2;
            color: %5;
            font-weight: bold;
            border-color: %1;
        }
        QTabBar::tab:hover {
            background-color: %6;
        }
    )").arg(sm.borderColour(),
                                   sm.tabSelected(),
                                   sm.windowBackground(),
                                   sm.tabUnselected(),
                                   sm.textColour(),
                                   sm.panelBackground()));

    // ── Status bar ────────────────────────────────────────────────────────────
    statusBar()->setStyleSheet(QString(
                                   "QStatusBar { background-color: %1; color: white; }"
                                   ).arg(sm.statusBackground()));
}

// ─────────────────────────────────────────────────────────────────────────────
// Slot implementations
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onNewQuote()
{
    if (m_quoteModified) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Unsaved Changes");
        msgBox.setText(
            "The current quote has unsaved changes.\n"
            "Save before creating a new quote?"
            );
        msgBox.setIcon(QMessageBox::Question);

        AnimatedButton *saveBtn   = new AnimatedButton("Save", &msgBox);
        saveBtn->setFixedSize(110, 40);
        AnimatedButton *discardBtn = new AnimatedButton("Discard", &msgBox);
        discardBtn->setFixedSize(110, 40);
        AnimatedButton *cancelBtn = new AnimatedButton("Cancel", &msgBox);
        cancelBtn->setFixedSize(110, 40);

        msgBox.addButton(saveBtn,    QMessageBox::YesRole);
        msgBox.addButton(discardBtn, QMessageBox::NoRole);
        msgBox.addButton(cancelBtn,  QMessageBox::RejectRole);
        msgBox.exec();

        QAbstractButton *clicked = msgBox.clickedButton();
        if (clicked == cancelBtn)
            return;
        if (clicked == saveBtn)
            saveCurrentQuote();
    }

    // Show quote type selector.
    QuoteTypeDialog typeDlg(this);
    if (typeDlg.exec() != QDialog::Accepted)
        return;

    newQuote(typeDlg.selectedType());
}

void MainWindow::onOpenQuote()
{
    // Phase 7 — not yet implemented
    statusBar()->showMessage("Open Quote — coming in Phase 7", 3000);
}

void MainWindow::onPreviewQuote()
{
    // Must have at least a site name before previewing.
    if (m_titleSection->siteName().isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Cannot Preview");
        msgBox.setText(
            "Please enter a Site / Project Name in the Title section "
            "before previewing the quote."
            );
        msgBox.setIcon(QMessageBox::Information);
        AnimatedButton *okBtn = new AnimatedButton("OK", &msgBox);
        okBtn->setFixedSize(110, 40);
        msgBox.addButton(okBtn, QMessageBox::AcceptRole);
        msgBox.exec();
        return;
    }

    // Save current state before previewing.
    saveCurrentQuote();

    // Get current price rows.
    QList<PriceRow> rows = m_priceSection->priceRows();

    // Open the preview dialog.
    QuotePreviewDialog dlg(m_currentQuote, rows, this);
    dlg.exec();
}

void MainWindow::onGeneratePdf()
{
    // Phase 6 — not yet implemented
    statusBar()->showMessage("Generate PDF — coming in Phase 6", 3000);
}

// void MainWindow::onSettings()
// {
//     // Open the config dialog for editing.
//     // Unlike first run, cancelling here is fine — we just don't save.
//     ConfigDialog dlg(this);
//     if (dlg.exec() == QDialog::Accepted) {
//         // Reload config and update the window title.
//         m_config = Database::loadConfig();
//         setWindowTitle("Quote Generation — " + m_config.companyName);
//         m_statusLabel->setText("  Status: " + m_currentQuote.status +
//                                "  |  Settings saved.");
//     }
// }

void MainWindow::onSettings()
{
    DropdownManagerDialog dlg(this);
    dlg.exec();
}

void MainWindow::onAbout()
{
    QMessageBox::about(
        this,
        "About Quote Generation",
        "<h3>Quote Generation v1.0.0</h3>"
        "<p>Professional quotation generation for small and medium businesses.</p>"
        "<p>Developed by PB Software Solutions</p>"
        "<p>Built with Qt " + QString(QT_VERSION_STR) + "</p>"
        );
}

void MainWindow::onTabChanged(int index)
{
    Q_UNUSED(index)
    // Tab changes don't need to update the status bar —
    // the permanent quote status label already shows what matters.
}

// ─────────────────────────────────────────────────────────────────────────────
// resizeEvent()
//
// Called automatically by Qt whenever the window is resized.
// We use it to rescale the logo banner to always fill the full width
// without distortion or blurring.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    // Retrieve the original unscaled pixmap we stored earlier.
    QVariant prop = m_logoBanner->property("originalPixmap");
    if (!prop.isNull()) {
        QPixmap original = prop.value<QPixmap>();
        if (!original.isNull()) {
            // Scale to the current label width, keeping aspect ratio sharp.
            m_logoBanner->setPixmap(
                original.scaled(
                    m_logoBanner->width(),
                    m_logoBanner->height(),
                    Qt::KeepAspectRatioByExpanding,
                    Qt::SmoothTransformation
                    )
                );
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// onToggleDarkMode()
//
// Toggles between light and dark mode, saves the preference to the
// database, and refreshes the entire application appearance.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onToggleDarkMode()
{
    // Toggle the mode in StyleManager.
    StyleManager::instance().toggle();
    bool isDark = StyleManager::instance().isDarkMode();

    // Save the preference to the database.
    AppConfig cfg = Database::loadConfig();
    cfg.darkMode = isDark;
    Database::saveConfig(cfg);

    // Update the menu item check state.
    m_actToggleDarkMode->setChecked(isDark);

    // Reapply styles to the entire application.
    StyleManager::instance().applyToApplication(qApp);

    // Reapply the toolbar and menu styles.
    applyStyleSheet();

    // Update the window palette directly.
    QPalette pal = palette();
    pal.setColor(QPalette::Window,
                 QColor(StyleManager::instance().windowBackground()));
    setPalette(pal);

    m_statusLabel->setText(
        isDark ? "  Dark mode enabled" : "  Light mode enabled"
        );
}

// ─────────────────────────────────────────────────────────────────────────────
// onQuoteDataChanged()
//
// Called whenever any section emits dataChanged.
// Marks the quote as modified and updates the status bar.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onQuoteDataChanged()
{
    m_quoteModified = true;
    QString status = m_currentQuote.status.isEmpty() ?
                         "Draft" : m_currentQuote.status;
    m_statusLabel->setText("  Status: " + status + "  |  Unsaved changes");
    m_saveDebounceTimer->start();
}

// ─────────────────────────────────────────────────────────────────────────────
// saveCurrentQuote()
//
// Gathers data from all sections and saves to the database.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::saveCurrentQuote()
{
    // Only save if the site name has been entered — minimum requirement.
    if (m_titleSection->siteName().isEmpty())
        return;

    // Gather data from all sections.
    m_currentQuote.quoteDate         = m_titleSection->quoteDate();
    m_currentQuote.siteName          = m_titleSection->siteName();
    m_currentQuote.titleText         = m_titleSection->quoteTitle();
    m_currentQuote.systemText        = m_systemSection->systemText();
    m_currentQuote.basisText         = m_basisSection->basisText();
    m_currentQuote.scopeText         = m_scopeSection->scopeText();
    m_currentQuote.exclusions        = m_exclusionsSection->exclusionsText();
    m_currentQuote.generalConditions = m_generalSection->generalText();
    m_currentQuote.clarifications    = m_clarificationsSection->clarificationsText();
    m_currentQuote.contactStatement  = m_signatureSection->contactStatement();
    m_currentQuote.signatoryName     = m_signatureSection->signatoryName();

    // Save to database.
    m_currentQuote = Database::saveQuote(m_currentQuote);

    // Save price items separately.
    if (m_currentQuote.id > 0) {
        Database::savePriceItems(m_currentQuote.id,
                                 m_priceSection->priceRows());
    }

    m_quoteModified = false;

    // Update status bar.
    m_statusLabel->setText(
        "  Status: " + m_currentQuote.status +
        "  |  Last saved: " + m_currentQuote.lastSaved
        );


    qDebug() << "Quote saved. ID:" << m_currentQuote.id;
}

// ─────────────────────────────────────────────────────────────────────────────
// onAutoSave()
//
// Called by the timer every 2 minutes.
// Only saves if there are unsaved changes.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::onAutoSave()
{
    if (m_quoteModified) {
        saveCurrentQuote();
         // Auto-save message is handled by saveCurrentQuote() updating the label.
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// loadQuote()
//
// Loads a quote from the database and populates all sections.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::loadQuote(int id)
{
    m_currentQuote = Database::loadQuote(id);

    if (m_currentQuote.id == 0)
        return;

    // Populate all sections with the loaded data.
    m_titleSection->loadData(
        m_currentQuote.quoteDate,
        m_currentQuote.titleText,
        m_currentQuote.siteName
        );
    m_systemSection->loadData(m_currentQuote.systemText);
    m_basisSection->loadData(m_currentQuote.basisText);
    m_scopeSection->loadData(m_currentQuote.scopeText);
    m_exclusionsSection->loadData(m_currentQuote.exclusions);
    m_generalSection->loadData(m_currentQuote.generalConditions);
    m_clarificationsSection->loadData(m_currentQuote.clarifications);
    m_signatureSection->loadData(
        m_currentQuote.contactStatement,
        m_currentQuote.signatoryName
        );

    // Load price items.
    QList<PriceRow> rows = Database::loadPriceItems(m_currentQuote.id);
    m_priceSection->loadData(rows);

    m_quoteModified = false;

    // Update window title and status bar.
    setWindowTitle("Quote Generation — " + m_config.companyName +
                   " — " + m_currentQuote.siteName);
    m_statusLabel->setText(
        "  Status: " + m_currentQuote.status +
        "  |  Last saved: " + m_currentQuote.lastSaved
        );
    // Restore quote type visibility in all sections.
    m_basisSection->setQuoteType(m_currentQuote.quoteType);
    m_scopeSection->setQuoteType(m_currentQuote.quoteType);
    m_exclusionsSection->setQuoteType(m_currentQuote.quoteType);
    m_generalSection->setQuoteType(m_currentQuote.quoteType);
    qDebug() << "Quote loaded. ID:" << m_currentQuote.id;
}

// ─────────────────────────────────────────────────────────────────────────────
// newQuote()
//
// Resets all sections ready for a new quote.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::newQuote(const QString &quoteType)
{
    m_currentQuote  = QuoteData();
    m_currentQuote.quoteType = quoteType;
    // Update all sections to show appropriate clause lists.
    m_basisSection->setQuoteType(quoteType);
    m_scopeSection->setQuoteType(quoteType);
    m_exclusionsSection->setQuoteType(quoteType);
    m_generalSection->setQuoteType(quoteType);
    m_quoteModified = false;

    // Reset all sections to empty.
    m_titleSection->loadData(
        QDate::currentDate().toString("dd/MM/yyyy"), "", ""
        );
    m_systemSection->loadData("");
    m_basisSection->loadData("");
    m_scopeSection->loadData("");
    m_exclusionsSection->loadData("");
    m_generalSection->loadData("");
    m_clarificationsSection->loadData("");
    m_signatureSection->loadData("", "");
    m_priceSection->loadData({});

    setWindowTitle("Quote Generation — " + m_config.companyName +
                   " — New Quote");
    m_statusLabel->setText("  Status: Draft  |  New quote");
}

// ─────────────────────────────────────────────────────────────────────────────
// closeEvent()
//
// Called when the user closes the application.
// Saves any unsaved changes before closing.
// ─────────────────────────────────────────────────────────────────────────────
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_quoteModified)
        saveCurrentQuote();

    event->accept();
}
