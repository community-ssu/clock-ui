#include <QtGui/QApplication>
#include "mainwindow.h"
#include "osso-intl.h"
#include <QFile>
#include <QTextStream>

#include <libosso.h>
#define VERSION "0.1"
#define SERVICE_NAME "com.nokia.worldclock"



//When starting another instance, instead raise this.
static gint rpc_callback(const gchar* interface, const gchar* method, GArray* arguments, gpointer user_data, osso_rpc_t* retval) {
	QMainWindow*  window;
	window = (QMainWindow*)user_data;

	if (!strcmp(interface, SERVICE_NAME) && !strcmp(method, "top_application")) {
		window->raise();
		window->activateWindow();
	}
	retval->type = DBUS_TYPE_INVALID;

	return OSSO_OK;
}


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

    //Application must register it, unless system kills it.
    osso_context_t* osso_context;
    osso_context = osso_initialize(SERVICE_NAME, VERSION, 0, NULL);

    //Prevent starting more than one instance of application
    g_assert(osso_rpc_set_default_cb_f(osso_context, rpc_callback, &w) == OSSO_OK);

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
