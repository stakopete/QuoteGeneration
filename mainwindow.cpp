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
    m_tabs->addTab(makePlaceholder("Scope of Works"),
                   "Scope");
    m_tabs->addTab(makePlaceholder("Exclusions"),
                   "Exclusions");
    m_tabs->addTab(makePlaceholder("General Conditions"),
                   "General");
    m_tabs->addTab(makePlaceholder("Clarifications"),
                   "Clarifications");
    m_tabs->addTab(makePlaceholder("Signature Block"),
                   "Signature");

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
    m_statusLabel = new QLabel("  No quote open");
    statusBar()->addWidget(m_statusLabel);
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
    // Phase 4 — not yet implemented
    statusBar()->showMessage("New Quote — coming in Phase 4", 3000);
}

void MainWindow::onOpenQuote()
{
    // Phase 7 — not yet implemented
    statusBar()->showMessage("Open Quote — coming in Phase 7", 3000);
}

void MainWindow::onPreviewQuote()
{
    // Phase 5 — not yet implemented
    statusBar()->showMessage("Preview — coming in Phase 5", 3000);
}

void MainWindow::onGeneratePdf()
{
    // Phase 6 — not yet implemented
    statusBar()->showMessage("Generate PDF — coming in Phase 6", 3000);
}

void MainWindow::onSettings()
{
    // Open the config dialog for editing.
    // Unlike first run, cancelling here is fine — we just don't save.
    ConfigDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        // Reload config and update the window title.
        m_config = Database::loadConfig();
        setWindowTitle("Quote Generation — " + m_config.companyName);
        statusBar()->showMessage("Settings saved.", 3000);
    }
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
    // Update the status bar to show which section is active.
    // We will expand this in Phase 4 to show section completion status.
    QStringList sections = {
        "Title & Date", "System Offered", "Basis of Proposal",
        "Proposed Price", "Scope of Works", "Exclusions",
        "General Conditions", "Clarifications", "Signature Block"
    };

    if (index >= 0 && index < sections.size()) {
        statusBar()->showMessage(
            "Section: " + sections[index], 2000
            );
    }
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

    statusBar()->showMessage(
        isDark ? "Dark mode enabled" : "Light mode enabled", 3000
        );
}
