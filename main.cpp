#include <QtGui/QApplication>
#include "mainwindow.h"
#include "osso-intl.h"
#include <QFile>
#include <QTextStream>

#ifdef Q_WS_MAEMO_5
#include <QDBusConnection>
#include <QDBusMessage>
#define DBUS_SERVICE "com.nokia.worldclock"
#define DBUS_PATH "/com/nokia/worldclock"
#define DBUS_INTERFACE "com.nokia.worldclock"
#endif

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

#ifdef Q_WS_MAEMO_5
    if ( ! QDBusConnection::sessionBus().registerService(DBUS_SERVICE) ) {
        QDBusConnection::sessionBus().send(QDBusMessage::createMethodCall(DBUS_SERVICE, DBUS_PATH, DBUS_INTERFACE, "top_application"));
        return 0;
    }
#endif

    MainWindow w;

#ifdef Q_WS_MAEMO_5
    if ( ! QDBusConnection::sessionBus().registerObject(DBUS_PATH, &w, QDBusConnection::ExportScriptableSlots) )
        return 1;
#endif

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
