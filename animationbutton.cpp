// ─────────────────────────────────────────────────────────────────────────────
// animatedbutton.cpp
//
// Implementation of the animated blue button.
// ─────────────────────────────────────────────────────────────────────────────

#include "animatedbutton.h"

#include <QMouseEvent>
#include <QSizePolicy>

// Path to the button background image.
// This is defined as a constant so it only needs changing in one place
// if the resources folder moves.
const QString AnimatedButton::BUTTON_IMAGE_PATH =
    "D:/Qt_Projects/QuoteGeneration/resources/buttons/bluebutton.png";

// ─────────────────────────────────────────────────────────────────────────────
// Constructors
// ─────────────────────────────────────────────────────────────────────────────
AnimatedButton::AnimatedButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
    applyStyle(false);
    setCursor(Qt::PointingHandCursor);  // Show hand cursor on hover
}

AnimatedButton::AnimatedButton(QWidget *parent)
    : QPushButton(parent)
{
    applyStyle(false);
    setCursor(Qt::PointingHandCursor);
}

// ─────────────────────────────────────────────────────────────────────────────
// applyStyle()
//
// Applies the button stylesheet. When pressed is true the button is
// drawn slightly smaller to simulate being pushed in. The size change
// is achieved using padding — increasing padding on all sides shrinks
// the visible content area without moving the button's position on screen.
// ─────────────────────────────────────────────────────────────────────────────
void AnimatedButton::applyStyle(bool pressed)
{
    // Normal state padding vs pressed state padding.
    // Adding 2px to all sides when pressed makes the button appear
    // to shrink inward without affecting its position in the layout.
    QString padding = pressed ? "6px 10px" : "4px 8px";
    QString fontSize = pressed ? "11px" : "12px";

    setStyleSheet(QString(
                      "QPushButton {"
                      "    color: white;"
                      "    font-weight: bold;"
                      "    font-size: %1;"
                      "    border: none;"
                      "    padding: %2;"
                      "    background-image: url('%3');"
                      "    background-repeat: no-repeat;"
                      "    background-position: center;"
                      "    background-attachment: fixed;"
                      "    border-radius: 4px;"
                      "}"
                      ).arg(fontSize, padding, BUTTON_IMAGE_PATH));
}

// ─────────────────────────────────────────────────────────────────────────────
// mousePressEvent()
//
// Called when the user presses the mouse button down over this widget.
// We apply the pressed style then pass the event to the base class
// so normal button behaviour (click signal etc.) still works.
// ─────────────────────────────────────────────────────────────────────────────
void AnimatedButton::mousePressEvent(QMouseEvent *event)
{
    applyStyle(true);
    QPushButton::mousePressEvent(event);
}

// ─────────────────────────────────────────────────────────────────────────────
// mouseReleaseEvent()
//
// Called when the user releases the mouse button.
// We revert to the normal style.
// ─────────────────────────────────────────────────────────────────────────────
void AnimatedButton::mouseReleaseEvent(QMouseEvent *event)
{
    applyStyle(false);
    QPushButton::mouseReleaseEvent(event);
}