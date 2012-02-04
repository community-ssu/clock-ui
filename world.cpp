#include "qabstracteventdispatcher.h"
#include "qmaemo5rotator.h"
#include "world.h"
#include "ui_world.h"
#include "osso-intl.h"
#include "dialog3.h"
#include "filedelegate.h"
#include "cityinfo.h"
#include "clockd/libtime.h"
#include <libosso.h>
#include <QDebug>
#include <QSettings>
// for strftime
#include <time.h>
// for setlocale
#include <locale.h>

static const char *getHildonTranslation(const char *string)
{
     setlocale (LC_ALL, "");
     const char *translation = ::dgettext("hildon-libs", string);
     if (qstrcmp(string, translation) == 0)
         return 0;
     return translation;
}

const char *hildonDateDayNameShrt = getHildonTranslation("wdgt_va_date_day_name_short");

static QString formatHildonDate(const QDateTime &dt, const char *format)
{
     if (!format)
         return QString();

     char buf[255];
     struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

     if (!dt.date().isNull()) {
         tm.tm_wday = dt.date().dayOfWeek() % 7;
         tm.tm_mday = dt.date().day();
         tm.tm_mon = dt.date().month() - 1;
         tm.tm_year = dt.date().year() - 1900;
     }
     if (!dt.time().isNull()) {
         tm.tm_sec = dt.time().second();
         tm.tm_min = dt.time().minute();
         tm.tm_hour = dt.time().hour();
     }

     size_t resultSize = ::strftime(buf, sizeof(buf), format, &tm);
     if (!resultSize)
         return QString();

     return QString::fromUtf8(buf, resultSize);
}

World::World(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::World)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setWindowFlags(Qt::Window);

    this->setWindowTitle(_("cloc_ti_world_clocks"));

    ui->pushButton->setIcon(QIcon::fromTheme("general_add"));
    ui->pushButton->setText(_("cloc_me_new_world_clock"));

    FileDelegate *pluginDelegate = new FileDelegate(ui->treeWidget);
    ui->treeWidget->setItemDelegate(pluginDelegate);

    loadCurrent();

    QSettings settings("cepiperez", "worldclock");
    QStringList listado = settings.value("Cities",QStringList()).toStringList();
    if ( listado.count() > 0 )
    {
        for (int i=0; i<listado.count(); ++i)
        {
            QString t = listado.at(i);
            addCity( t.toInt() );
        }
    }

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

}

World::~World()
{
    delete ui;
}

QString World::longdate(QString data)
{
    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();

    if ( (data.contains(localAMtxt)) || (data.contains(localPMtxt)) )
        return localAMtxt;
/*    else if ( (data.contains("a.m.")) || (data.contains("p.m.")) )
        return "a.m.";
    else if ( (data.contains("AM")) || (data.contains("PM")) )
        return "AM"; */
    else
        return "no";
}

void World::orientationChanged()
{
    int len = 0;
    if ( ui->treeWidget->topLevelItemCount() > 0 )
    {
        if ( longdate(ui->treeWidget->topLevelItem(0)->text(0)) != "no" )
            len = 26;
    }

    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height())
    {
        ui->treeWidget->header()->resizeSection(0,116+len);
        ui->treeWidget->header()->resizeSection(1,210-len);
        ui->treeWidget->header()->resizeSection(2,100);
    } else {
        ui->treeWidget->header()->resizeSection(0,116+len);
        ui->treeWidget->header()->resizeSection(1,424-len);
        ui->treeWidget->header()->resizeSection(2,206);
    }
    ui->treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

void World::on_pushButton_pressed()
{
    Dialog3* hw = new Dialog3(this);
    hw->exec();
    if ( hw->selected != "" )
    {
        addCity(hw->selected.toInt());

        QSettings settings("cepiperez", "worldclock");
        QStringList listado = settings.value("Cities",QStringList()).toStringList();
        listado.append(hw->selected);
        settings.setValue("Cities", listado);
        settings.sync();
    }
    delete hw;
}

void World::addCity(int cityId)
{
    QTreeWidgetItem *pepe = new QTreeWidgetItem();

    Cityinfo* city = cityinfo_from_id(cityId);

    QString n = QString::fromUtf8(city->name) + "  startdesc" + QString::fromUtf8(city->country);
    pepe->setText(1, n);
    pepe->setWhatsThis(1, "world-name");

    QString timeoffset;
    int offset = time_get_utc_offset(cityinfo_get_zone(city));
    QString sign;
    if (offset > 0)
        sign = "";
    else
        sign = "+";
    if((offset % 3600)==0)
        timeoffset = QString("GMT %3").arg(sign+QString::number(-offset/3600));
    else
        timeoffset = QString("GMT %3:%4").arg(sign+QString::number(-offset/3600)).arg("30");

    QDateTime tiempo = QDateTime::currentDateTime();

    tiempo = tiempo.toUTC();
    tiempo = tiempo.addSecs( -offset );

    pepe->setText(0, QLocale::system().toString( tiempo.time(), QLocale::ShortFormat) );
    pepe->setWhatsThis(0, "time");
    QString CurrentdayNameShort = formatHildonDate(tiempo, hildonDateDayNameShrt);
    QStringList sl = CurrentdayNameShort.split(' ', QString::SkipEmptyParts);
    CurrentdayNameShort = sl.at(0);
    QString LocalDateShort = tiempo.date().toString(Qt::DefaultLocaleShortDate);
    QRegExp LongYearEnd( "\\d{4}$" ); 
    /* we have 4 digits at the end of the date, meaning yyyy
       next we replace this by the last two digits (yy), so no century info */
    if (LongYearEnd.indexIn(LocalDateShort) != -1 )
  	LocalDateShort.replace(LongYearEnd, LocalDateShort.right(2));
    QRegExp LongYearBegin( "^\\d{4}" ); 
    /* we have 4 digits at the beginning of the date, meaning yyyy
       next we replace this by the last two digits (yy), so no century info */
    if (LongYearBegin.indexIn(LocalDateShort) != -1 )
  	LocalDateShort.replace(LongYearBegin, LocalDateShort.mid(2,2));

    pepe->setText(2, CurrentdayNameShort + " "
                  + LocalDateShort
                  + "  startdesc" + timeoffset );
    pepe->setWhatsThis(2, "world-date");

    pepe->setStatusTip(0, QString::number(cityId));
    pepe->setStatusTip(1, QString::number(offset));

    ui->treeWidget->addTopLevelItem(pepe);

    cityinfo_free(city);
}

void World::loadCurrent()
{
    int ret = 0;
    char current_tz[32];
    QString timeoffset;
    ret = time_get_timezone(&current_tz[0], 32);
    if(ret>0 && ret < 32)
    {
      Cityinfo** cities = cityinfo_get_all();
      Cityinfo** cities_iter = cities;
      bool current_found = false;
      while(*cities_iter && !current_found)
      {
        char* tz = cityinfo_get_zone(*cities_iter);

        if(g_strcmp0(tz, current_tz+1)==0)
        {
          current_found = true;

          QTreeWidgetItem *pepe = new QTreeWidgetItem();

          QTime tiempo = QTime::currentTime();
          pepe->setText(0, QLocale::system().toString( tiempo, QLocale::ShortFormat) );
          pepe->setWhatsThis(0, "time");

          pepe->setText(1, _("cloc_fi_local_time") + " startdesc (" +
                        QString::fromUtf8(cityinfo_clone(*cities_iter)->name) + ", " +
                        QString::fromUtf8(cityinfo_clone(*cities_iter)->country) + ")" );
          pepe->setWhatsThis(1, "world-name");

          int offset = time_get_utc_offset(cityinfo_get_zone(*cities_iter));
          QString sign;
          if (offset > 0)
              sign = "";
          else
              sign = "+";
          if((offset % 3600)==0)
              timeoffset = QString("GMT %3").arg(sign+QString::number(-offset/3600));
          else
              timeoffset = QString("GMT %3:%4").arg(sign+QString::number(-offset/3600)).arg("30");

          QDate fecha = QDate::currentDate();
          QDateTime date_time = QDateTime::currentDateTime();
          /* Local time */
          QString CurrentdayNameShort = formatHildonDate(date_time, hildonDateDayNameShrt);
          QStringList sl = CurrentdayNameShort.split(' ', QString::SkipEmptyParts);
          CurrentdayNameShort = sl.at(0);

          QString LocalDateShort = fecha.toString(Qt::DefaultLocaleShortDate);
          QRegExp LongYearEnd( "\\d{4}$" ); 
          /* we have 4 digits at the end of the date, meaning yyyy
             next we replace this by the last two digits (yy), so no century info */
          if (LongYearEnd.indexIn(LocalDateShort) != -1 )
              LocalDateShort.replace(LongYearEnd, LocalDateShort.right(2));
          QRegExp LongYearBegin( "^\\d{4}" ); 
          /* we have 4 digits at the beginning of the date, meaning yyyy
             next we replace this by the last two digits (yy), so no century info */
          if (LongYearBegin.indexIn(LocalDateShort) != -1 )
              LocalDateShort.replace(LongYearBegin, LocalDateShort.mid(2,2));

          pepe->setText(2, CurrentdayNameShort + " "
                        + LocalDateShort
                        + "  startdesc" + timeoffset );

          pepe->setWhatsThis(2, "world-date");
          QString tr = timeoffset;
          tr.remove("GMT ");
          curTime = tr.toInt();

          ui->treeWidget->addTopLevelItem(pepe);

        }
        ++cities_iter;
      }
      cityinfo_free_all(cities);
    }

    line1 = timeoffset;

}

void World::on_treeWidget_customContextMenuRequested(QPoint pos)
{
    if ( ( ui->treeWidget->topLevelItemCount() == 1 ) ||
         ( ui->treeWidget->currentItem()->text(1).contains(_("cloc_fi_local_time")) ) )
        return;

    intl("rtcom-call-ui");
    QMenu *contextMenu = new QMenu(this);
    contextMenu->addAction(QIcon(), _("call_me_context_menu_delete"), this, SLOT(removeSel()));
    intl("osso-clock");
    contextMenu->exec(mapToGlobal(pos));

}

void World::removeSel()
{
    ui->treeWidget->takeTopLevelItem( ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->currentItem()) );
    QStringList listado;
    if ( ui->treeWidget->topLevelItemCount() > 1 )
    {
        for (int i=0; i<ui->treeWidget->topLevelItemCount()-1; ++i)
            listado.append( ui->treeWidget->topLevelItem(i+1)->statusTip(0) );
    }
    QSettings settings("cepiperez", "worldclock");
    settings.setValue("Cities", listado);
    settings.sync();

}

void World::updateClocks()
{
    QDateTime tiempo = QDateTime::currentDateTime();
    ui->treeWidget->topLevelItem(0)->setText(0, QLocale::system().toString( tiempo.time(), QLocale::ShortFormat) );

    if ( ui->treeWidget->topLevelItemCount() > 1 )
    {
        for (int i=0; i<ui->treeWidget->topLevelItemCount()-1; ++i)
        {
            int off = ui->treeWidget->topLevelItem(i+1)->statusTip(1).toInt();
            tiempo = QDateTime::currentDateTime();
            tiempo = tiempo.toUTC();
            tiempo = tiempo.addSecs( -off );
            ui->treeWidget->topLevelItem(i+1)->setText(0, QLocale::system().toString( tiempo.time(), QLocale::ShortFormat) );
        }

    }


}
