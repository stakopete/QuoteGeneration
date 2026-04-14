#ifndef QUOTETYPEDIALOG_H
#define QUOTETYPEDIALOG_H

// ─────────────────────────────────────────────────────────────────────────────
// quotetypedialog.h
//
// Dialog shown when the user creates a new quote.
// Asks what type of quote is being prepared so the application can
// show the appropriate clause lists and format the preview correctly.
// ─────────────────────────────────────────────────────────────────────────────

#include <QDialog>

class QRadioButton;
class QLabel;
class AnimatedButton;

class QuoteTypeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QuoteTypeDialog(QWidget *parent = nullptr);

    // Returns the selected quote type string.
    // One of: "Wet", "Dry", "Combined", "General"
    QString selectedType() const;

private slots:
    void onProceed();

private:
    void setupUi();

    QRadioButton   *m_wetRadio;
    QRadioButton   *m_dryRadio;
    QRadioButton   *m_combinedRadio;
    QRadioButton   *m_generalRadio;
    AnimatedButton *m_proceedButton;
    AnimatedButton *m_cancelButton;
};

#endif // QUOTETYPEDIALOG_H