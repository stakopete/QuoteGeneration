// ─────────────────────────────────────────────────────────────────────────────
// stylemanager.cpp
// ─────────────────────────────────────────────────────────────────────────────

#include "stylemanager.h"
#include <QApplication>
#include <QPalette>

// ─────────────────────────────────────────────────────────────────────────────
// instance()
//
// Returns the single global StyleManager instance.
// The 'static' keyword here means Qt creates it once and reuses it.
// ─────────────────────────────────────────────────────────────────────────────
StyleManager &StyleManager::instance()
{
    static StyleManager inst;
    return inst;
}

// ─────────────────────────────────────────────────────────────────────────────
// setDarkMode()
// ─────────────────────────────────────────────────────────────────────────────
void StyleManager::setDarkMode(bool dark)
{
    m_darkMode = dark;
}

// ─────────────────────────────────────────────────────────────────────────────
// Colour definitions
//
// Each method returns the appropriate colour for the current mode.
// Light mode uses warm neutral tones.
// Dark mode uses dark greys — not pure black which is too harsh.
// ─────────────────────────────────────────────────────────────────────────────

QString StyleManager::windowBackground() const
{
    return m_darkMode ? "#1e1e1e" : "#c0c0c0";
}

QString StyleManager::panelBackground() const
{
    return m_darkMode ? "#2d2d2d" : "#d4d4d4";
}

QString StyleManager::inputBackground() const
{
    // Light mode: warm parchment #c2bca0 as requested.
    // Dark mode: dark input field.
    return m_darkMode ? "#3c3c3c" : "#ece9dc";
}

QString StyleManager::textColour() const
{
    return m_darkMode ? "#e0e0e0" : "#1a1a1a";
}

QString StyleManager::labelColour() const
{
     return m_darkMode ? "#e0e0e0" : "#1a1a1a";
}

QString StyleManager::borderColour() const
{
    return m_darkMode ? "#555555" : "#999999";
}

QString StyleManager::headerBackground() const
{
    return m_darkMode ? "#1a1a2e" : "#2c3e50";
}

QString StyleManager::headerText() const
{
    return "#ffffff";   // Always white regardless of mode
}

QString StyleManager::selectionBackground() const
{
    return m_darkMode ? "#4a6278" : "#3d5166";
}

QString StyleManager::selectionText() const
{
    return "#ffffff";
}

QString StyleManager::tabSelected() const
{
    return m_darkMode ? "#2d2d2d" : "#d4d4d4";
}

QString StyleManager::tabUnselected() const
{
    return m_darkMode ? "#1e1e1e" : "#a8a8a8";
}

QString StyleManager::menuBackground() const
{
    return m_darkMode ? "#1a1a2e" : "#2c3e50";
}

QString StyleManager::toolbarBackground() const
{
    return m_darkMode ? "#252535" : "#34495e";
}

QString StyleManager::statusBackground() const
{
    return m_darkMode ? "#1a1a2e" : "#2c3e50";
}

// ─────────────────────────────────────────────────────────────────────────────
// Stylesheet generators
// ─────────────────────────────────────────────────────────────────────────────

QString StyleManager::groupBoxStyle() const
{
    return QString(R"(
        QGroupBox {
            font-weight: bold;
            color: %1;
            border: 1px solid %2;
            border-radius: 4px;
            margin-top: 16px;
            padding-top: 12px;
            background-color: %3;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 4px;
            color: %1;
            background-color: transparent;
        }
        QLabel {
            color: %1;
            background-color: transparent;
        }
        QListWidget {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
        }
        QListWidget::item:selected {
            background-color: %5;
            color: white;
        }
        QListWidget::item:hover {
            background-color: %3;
        }
QLineEdit {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
            padding: 4px;
            font-weight: bold;
        }
        QTextEdit {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
            padding: 4px;
            font-weight: bold;
        }
        QComboBox {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
            padding: 3px;
        }
        QComboBox QAbstractItemView {
            color: %1;
            background-color: %4;
            selection-background-color: %5;
            selection-color: white;
            border: 1px solid %2;
        }
        QComboBox::drop-down {
            width: 20px;
        }
        QDateEdit {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
            padding: 3px;
        }
        QSpinBox {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
            padding: 3px;
        }
        QDoubleSpinBox {
            color: %1;
            background-color: %4;
            border: 1px solid %2;
            border-radius: 3px;
            padding: 3px;
        }
    )")
        .arg(labelColour(),         // %1 - text/label colour
             borderColour(),        // %2 - border colour
             panelBackground(),     // %3 - group box background
             inputBackground(),     // %4 - input field background
             selectionBackground()  // %5 - selection highlight
             );
}

QString StyleManager::headingLabelStyle() const
{
    return QString(
               "QLabel {"
               "    background-color: %1;"
               "    color: %2;"
               "    padding: 8px 12px;"
               "    border-radius: 4px;"
               "}"
               ).arg(headerBackground(), headerText());
}

QString StyleManager::listWidgetStyle() const
{
    return QString(R"(
        QListWidget {
            color: %1;
            background-color: %2;
            border: 1px solid %3;
            border-radius: 3px;
        }
        QListWidget::item:selected {
            background-color: %4;
            color: white;
        }
        QListWidget::item:hover {
            background-color: %5;
        }
        QScrollBar:vertical {
            background: %5;
            width: 16px;
            border: 1px solid %3;
        }
        QScrollBar::handle:vertical {
            background: %4;
            min-height: 20px;
            border-radius: 2px;
        }
        QScrollBar::add-line:vertical {
            background: %5;
            height: 16px;
            subcontrol-position: bottom;
            subcontrol-origin: margin;
        }
        QScrollBar::sub-line:vertical {
            background: %5;
            height: 16px;
            subcontrol-position: top;
            subcontrol-origin: margin;
        }
        QScrollBar::up-arrow:vertical,
        QScrollBar::down-arrow:vertical {
            width: 0;
            height: 0;
        }
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: none;
        }
    )")
        .arg(textColour(),
             inputBackground(),
             borderColour(),
             selectionBackground(),
             panelBackground()
             );
}

QString StyleManager::textEditStyle() const
{
    return QString(
               "QTextEdit {"
               "    color: %1;"
               "    background-color: %2;"
               "    border: 1px solid %3;"
               "    border-radius: 3px;"
               "    padding: 4px;"
               "    font-weight: bold;"
               "}"
               ).arg(textColour(), inputBackground(), borderColour());
}

QString StyleManager::lineEditStyle() const
{
    return QString(
               "QLineEdit {"
               "    color: %1;"
               "    background-color: %2;"
               "    border: 1px solid %3;"
               "    border-radius: 3px;"
               "    padding: 4px;"
               "    font-weight: bold;"
               "}"
               ).arg(textColour(), inputBackground(), borderColour());
}

QString StyleManager::comboBoxStyle() const
{
    return QString(R"(
        QComboBox {
            color: %1;
            background-color: %2;
            border: 1px solid %3;
            border-radius: 3px;
            padding: 3px;
        }
        QComboBox QAbstractItemView {
            color: %1;
            background-color: %2;
            selection-background-color: %4;
            selection-color: white;
            border: 1px solid %3;
        }
        QComboBox::drop-down {
            width: 20px;
        }
    )")
        .arg(textColour(),
             inputBackground(),
             borderColour(),
             selectionBackground());
}

QString StyleManager::dateEditStyle() const
{
    return QString(
               "QDateEdit {"
               "    color: %1;"
               "    background-color: %2;"
               "    border: 1px solid %3;"
               "    border-radius: 3px;"
               "    padding: 3px;"
               "}"
               ).arg(textColour(), inputBackground(), borderColour());
}

// ─────────────────────────────────────────────────────────────────────────────
// applyToApplication()
//
// Sets the application-wide palette. This affects widgets that don't
// have explicit stylesheets set on them.
// ─────────────────────────────────────────────────────────────────────────────
void StyleManager::applyToApplication(QApplication *app)
{
    QPalette pal;
// Trying this colour ece9dc in place of c2bca0
    if (m_darkMode) {
        pal.setColor(QPalette::Window,          QColor("#1e1e1e"));
        pal.setColor(QPalette::WindowText,      QColor("#e0e0e0"));
        pal.setColor(QPalette::Base,            QColor("#2d2d2d"));
        pal.setColor(QPalette::AlternateBase,   QColor("#3c3c3c"));
        pal.setColor(QPalette::Text,            QColor("#e0e0e0"));
        pal.setColor(QPalette::Button,          QColor("#2d2d2d"));
        pal.setColor(QPalette::ButtonText,      QColor("#e0e0e0"));
        pal.setColor(QPalette::Highlight,       QColor("#4a6278"));
        pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
        pal.setColor(QPalette::ToolTipBase,     QColor("#2d2d2d"));
        pal.setColor(QPalette::ToolTipText,     QColor("#e0e0e0"));
    } else {
        pal.setColor(QPalette::Window,          QColor("#c0c0c0"));
        pal.setColor(QPalette::WindowText,      QColor("#2c3e50"));
        pal.setColor(QPalette::Base,            QColor("#c2bca0"));
        pal.setColor(QPalette::AlternateBase,   QColor("#d4d4d4"));
        pal.setColor(QPalette::Text,            QColor("#2c3e50"));
        pal.setColor(QPalette::Button,          QColor("#d4d4d4"));
        pal.setColor(QPalette::ButtonText,      QColor("#2c3e50"));
        pal.setColor(QPalette::Highlight,       QColor("#3d5166"));
        pal.setColor(QPalette::HighlightedText, QColor("#ffffff"));
        pal.setColor(QPalette::ToolTipBase,     QColor("#d4d4d4"));
        pal.setColor(QPalette::ToolTipText,     QColor("#2c3e50"));
    }

    app->setPalette(pal);
}