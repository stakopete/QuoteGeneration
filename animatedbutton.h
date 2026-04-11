#ifndef ANIMATEDBUTTON_H
#define ANIMATEDBUTTON_H

// ─────────────────────────────────────────────────────────────────────────────
// animatedbutton.h
//
// A custom push button that uses bluebutton.png as its background and
// animates by shrinking slightly on mouse press and reverting on release.
//
// This class is used for ALL buttons throughout the application to give
// a consistent look and feel as specified.
//
// Usage:
//   AnimatedButton *btn = new AnimatedButton("Button Text", parent);
//   connect(btn, &AnimatedButton::clicked, this, &MyClass::mySlot);
// ─────────────────────────────────────────────────────────────────────────────

#include <QPushButton>

class AnimatedButton : public QPushButton
{
    Q_OBJECT

public:
    // Text is the label shown on the button in white bold text.
    // Parent is the owning widget.
    explicit AnimatedButton(const QString &text,
                            QWidget *parent = nullptr);

    // Convenience constructor with no text — text can be set later
    // via setText().
    explicit AnimatedButton(QWidget *parent = nullptr);

protected:
    // These override the default mouse press/release handlers
    // to implement the animation effect.
    void mousePressEvent(QMouseEvent *event)   override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void applyStyle(bool pressed);

    // Path to the button background image.
    static const QString BUTTON_IMAGE_PATH;
};

#endif // ANIMATEDBUTTON_H