#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setStyle("Fusion");

    QFile style(":/style.qss");

    if (style.open(QIODevice::ReadOnly))
        a.setStyleSheet(style.readAll());

    MainWindow w;
    w.show();
    return QApplication::exec();
}
