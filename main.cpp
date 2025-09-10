#include "mainwindow.h"

#include <QApplication>
#include <QApplication>
#include <QPalette>
#include <QFont>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QFont appFont("Segoe Print", 14);
    appFont. setBold(10);
    app.setFont(appFont);

    MainWindow w;
    w.show();
    return app.exec();
}
