#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // The old stylesheet loading code is now REMOVED.
    // The new style is set inside the MainWindow constructor.

    MainWindow w;
    w.show();
    return a.exec();
}
