#include <QtGui/QApplication>
#include "mainwindow.h"
#include "osso-intl.h"
#include <QFile>
#include <QTextStream>


int main(int argc, char *argv[])
{
    intl("osso-clock");
    QApplication a(argc, argv);

    QString line, lang;
    QFile data( "/etc/osso-af-init/locale" );
    if (data.open(QFile::ReadOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        while ( !out.atEnd() )
        {
            line = out.readLine();
            if ( line.indexOf("export LANG") == 0 )
            {
                lang = line;
                lang.replace("export LANG=","");
            }
        }
    }
    data.close();
    QLocale::setDefault(lang);

    MainWindow w;



#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
