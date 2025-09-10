#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>
#include <QWidget>

namespace AppStyle {

// Применить общий стиль ко всему приложению
void applyAppStyle(QWidget *root);

// Вернуть стили для отдельных виджетов
QString groupBoxStyle();
QString buttonStyle();
QString sliderStyle();
QString spinBoxStyle();
QString formLabelStyle();

}

#endif // APPSTYLE_H
