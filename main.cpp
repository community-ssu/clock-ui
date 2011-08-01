#include <QtGui/QApplication>
#include "mainwindow.h"
#include "osso-intl.h"

int main(int argc, char *argv[])
{

    intl("osso-clock");

    QApplication a(argc, argv);
    MainWindow w;
#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
