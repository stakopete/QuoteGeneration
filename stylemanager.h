#ifndef STYLEMANAGER_H
#define STYLEMANAGER_H

// ─────────────────────────────────────────────────────────────────────────────
// stylemanager.h
//
// Central style management for the application.
// All colours and stylesheets are defined here — no hardcoded colours
// anywhere else in the application.
//
// Usage:
//   StyleManager::instance().setDarkMode(true);
//   QString sheet = StyleManager::instance().groupBoxStyle();
// ─────────────────────────────────────────────────────────────────────────────

#include <QString>
#include <QColor>

class QApplication;

class StyleManager
{
public:
    // ── Singleton access ──────────────────────────────────────────────────────
    // There is only ever one StyleManager. Access it via instance().
    static StyleManager &instance();

    // ── Mode control ─────────────────────────────────────────────────────────
    void setDarkMode(bool dark);
    bool isDarkMode() const { return m_darkMode; }
    void toggle() { setDarkMode(!m_darkMode); }

    // ── Colour access ─────────────────────────────────────────────────────────
    // These return the correct colour for the current mode.
    QString windowBackground()  const;
    QString panelBackground()   const;
    QString inputBackground()   const;  // #c2bca0 light / #3c3c3c dark
    QString textColour()        const;
    QString labelColour()       const;
    QString borderColour()      const;
    QString headerBackground()  const;
    QString headerText()        const;
    QString selectionBackground() const;
    QString selectionText()     const;
    QString tabSelected()       const;
    QString tabUnselected()     const;
    QString menuBackground()    const;
    QString toolbarBackground() const;
    QString statusBackground()  const;

    // ── Stylesheet generators ─────────────────────────────────────────────────
    // These return complete QSS stylesheets for specific widget types.
    // Call these in each section's setupUi() instead of hardcoding colours.

    // For QGroupBox containers.
    QString groupBoxStyle() const;

    // For section heading labels (the dark blue banner at top of each section).
    QString headingLabelStyle() const;

    // For QListWidget.
    QString listWidgetStyle() const;

    // For QTextEdit.
    QString textEditStyle() const;

    // For QLineEdit.
    QString lineEditStyle() const;

    // For QComboBox.
    QString comboBoxStyle() const;

    // For QDateEdit.
    QString dateEditStyle() const;

    // Applies the top-level application palette.
    // Call this once from main() after creating QApplication,
    // and again whenever the mode changes.
    void applyToApplication(QApplication *app);

private:
    // Private constructor — use instance() instead.
    StyleManager() = default;

    bool m_darkMode = false;
};

#endif // STYLEMANAGER_H