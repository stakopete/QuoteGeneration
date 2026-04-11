#ifndef TITLESECTION_H
#define TITLESECTION_H

// ─────────────────────────────────────────────────────────────────────────────
// titlesection.h
//
// The Title section widget. This is the first tab in the quote editor.
// It collects the quote date, title, and site/project name.
//
// Each section is a QWidget subclass. The tab widget in MainWindow
// holds one instance of each section widget.
// ─────────────────────────────────────────────────────────────────────────────

#include <QWidget>
#include "database.h"
#include "animatedbutton.h"

// Forward declarations
class QLineEdit;
class QComboBox;
class QDateEdit;
class QLabel;
//class QPushButton;

class TitleSection : public QWidget
{
    Q_OBJECT

public:
    explicit TitleSection(QWidget *parent = nullptr);

    // ── Data access ───────────────────────────────────────────────────────────
    // These are called by the main window when building the quote preview
    // or generating the PDF.

    // Returns the quote date formatted as dd/MM/yyyy.
    QString quoteDate() const;

    // Returns the selected or typed quote title.
    QString quoteTitle() const;

    // Returns the site/project name.
    QString siteName() const;

    // Returns true if all required fields are filled in.
    // Called before allowing the user to preview or generate the PDF.
    bool isComplete() const;

    // Loads previously saved data back into the fields.
    // Called when an existing quote is opened.
    void loadData(const QString &date,
                  const QString &title,
                  const QString &site);

signals:
    // Emitted whenever any field in this section changes.
    // MainWindow connects to this to trigger auto-save.
    void dataChanged();

private slots:
    void onTitleSelectionChanged(int index);
    void onAddCustomTitle();

private:
    void setupUi();
    void populateTitleDropdown();

    // ── Controls ──────────────────────────────────────────────────────────────
    QDateEdit   *m_dateEdit;
    QComboBox   *m_titleCombo;
    QLineEdit   *m_customTitle;     // Shown only when user selects "Other"
    QLineEdit   *m_siteName;
    AnimatedButton *m_addTitleButton;

    // Tracks whether the user is typing a custom title.
    bool m_usingCustomTitle = false;
};

#endif // TITLESECTION_H