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
    // When pressed we scale the button down using a transform.
    // This shrinks the whole button visually without affecting layout.
    if (pressed) {
        setStyleSheet(QString(
                          "QPushButton {"
                          "    color: white;"
                          "    font-weight: bold;"
                          "    font-size: 11px;"
                          "    border: none;"
                          "    padding: 4px 8px;"
                          "    border-image: url('%1');"
                          "    background-repeat: no-repeat;"
                          "    background-position: center;"
                          "    background-size: contain;"
                          "    border-radius: 4px;"
                          "    margin: 2px;"
                          "}"
                          ).arg(BUTTON_IMAGE_PATH));
    } else {
        setStyleSheet(QString(
                          "QPushButton {"
                          "    color: white;"
                          "    font-weight: bold;"
                          "    font-size: 12px;"
                          "    border: none;"
                          "    padding: 4px 8px;"
                          "    border-image: url('%1');"
                          "    background-repeat: no-repeat;"
                          "    background-position: center;"
                          "    background-size: contain;"
                          "    border-radius: 4px;"
                          "    margin: 0px;"
                          "}"
                          ).arg(BUTTON_IMAGE_PATH));
    }
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