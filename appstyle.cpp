#include "AppStyle.h"
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QApplication>
#include <QLabel>

namespace AppStyle {

static const char* COLOR_BG      = "#F7F7FA";  // общий фон
static const char* COLOR_BORDER  = "#C9CCD6";  // рамка
static const char* COLOR_TEXT    = "#04070b";  // текст
static const char* COLOR_ACCENT1 = "#6c91c1";
static const char* COLOR_ACCENT2 = "#A1B4D9";

QString groupBoxStyle() {
    return QString(R"(
        QGroupBox {
            border: 3px solid %1;
            border-radius: 10px;
            margin-top: 30px;
            background: #FFFFFF;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top center;
            padding: 4px 8px;
            font-weight: 600;
            font-size: 16pt;
            color: %2;
        }
    )").arg(COLOR_BORDER, COLOR_TEXT);
}

QString buttonStyle() {
    return QString(R"(
        QPushButton {
            background-color: %1;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 6px 14px;
            font-weight: 500;
        }
        QPushButton:hover { background-color: %2; }
        QPushButton:pressed { background-color: %3; }
    )").arg(COLOR_ACCENT1, COLOR_ACCENT2, COLOR_BORDER);
}

QString sliderStyle() {
    return QString(R"(
        QSlider::groove:horizontal {
            height: 6px;
            background: %1;
            border-radius: 3px;
        }
        QSlider::sub-page:horizontal {
            background: %2;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            width: 16px;
            height: 16px;
            margin: -5px 0;
            border-radius: 8px;
            background: #FFFFFF;
            border: 2px solid %2;
        }
    )").arg(COLOR_BORDER, COLOR_ACCENT2);
}

QString labelStyle() {
    return QString(R"(
        QLabel {
            color: %1;
            font-size: 10pt;
            font-weight: 500;
        }
    )").arg(COLOR_TEXT);
}

QString formLabelStyle() {
    return QString(R"(
        QLabel {
            font-size: 13pt;
            font-weight: bold;
            color: #004B6A;
        }
    )");
}

void applyAppStyle(QWidget *root) {
    if (!root) return;

    root->setStyleSheet(QString("background: %1;").arg(COLOR_BG));

    const auto widgets = root->findChildren<QWidget*>();
    for (QWidget *w : widgets) {
        if (qobject_cast<QGroupBox*>(w)) {
            w->setStyleSheet(groupBoxStyle());
        } else if (qobject_cast<QPushButton*>(w)) {
            w->setStyleSheet(buttonStyle());
        } else if (qobject_cast<QSlider*>(w)) {
            w->setStyleSheet(sliderStyle());
        } else if (auto *lbl = qobject_cast<QLabel*>(w)) {
            if (lbl->objectName() == "formLabel")
                lbl->setStyleSheet(formLabelStyle());
            else
                lbl->setStyleSheet(labelStyle());
        } else if (qobject_cast<QFrame*>(w) && w->objectName() == "previewFrame") {
            w->setStyleSheet(QString("border:1px solid %1; border-radius:12px;").arg(COLOR_BORDER));
        }
    }
}

}
