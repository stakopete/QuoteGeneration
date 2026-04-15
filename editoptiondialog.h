#ifndef EDITOPTIONDIALOG_H
#define EDITOPTIONDIALOG_H

// ---------------------------------------------------------------------------
// EditOptionDialog
// ---------------------------------------------------------------------------
// A minimal modal dialog used for both "Add new option" and "Edit existing
// option" operations inside the Dropdown Manager.
//
// It contains:
//   - A descriptive label (set by the caller)
//   - A QLineEdit for the option text
//   - OK and Cancel buttons (AnimatedButton)
//
// The caller reads the entered text via text() after exec() returns Accepted.
// ---------------------------------------------------------------------------

#include <QDialog>
#include <QString>

// Forward declarations — avoids pulling in full headers here.
// The .cpp file includes the full headers it needs.
class QLabel;
class QLineEdit;
class QVBoxLayout;
class AnimatedButton;

class EditOptionDialog : public QDialog
{
    Q_OBJECT

public:
    // -----------------------------------------------------------------------
    // Constructor
    //   parent      — owning widget (for correct modal behaviour and centering)
    //   title       — window title, e.g. "Add Option" or "Edit Option"
    //   label       — descriptive text shown above the input field
    //   initialText — pre-filled text (empty for Add, existing text for Edit)
    // -----------------------------------------------------------------------
    explicit EditOptionDialog(QWidget *parent,
                              const QString &title,
                              const QString &label,
                              const QString &initialText = QString());

    // Returns whatever the user typed in the input field.
    // Only meaningful when exec() returned QDialog::Accepted.
    QString text() const;

private slots:
    // Called when the user clicks OK — validates input is not empty before
    // accepting, so the caller never receives a blank string.
    void onOkClicked();

private:
    // -----------------------------------------------------------------------
    // Helpers
    // -----------------------------------------------------------------------
    void buildUi(const QString &label, const QString &initialText);
    void applyStyles();

    // -----------------------------------------------------------------------
    // Member variables
    // -----------------------------------------------------------------------
    QLineEdit      *m_lineEdit  = nullptr;
    AnimatedButton *m_okButton  = nullptr;
    AnimatedButton *m_cancelButton = nullptr;
};

#endif // EDITOPTIONDIALOG_H
