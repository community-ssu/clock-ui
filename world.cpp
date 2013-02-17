#include "qabstracteventdispatcher.h"
#include "world.h"
#include "ui_world.h"
#include "osso-intl.h"
#include "dialog3.h"
#include "citydetail.h"
#include "filedelegate.h"
#include "cityinfo.h"
#include "clockd/libtime.h"
#include "gconfitem.h"
#include "mainwindow.h"
#include <QDebug>
#include <QSettings>
#include <QDateTime>
#include <QDesktopWidget>
#include <QMenu>
#include <QProcess>
#include <dlfcn.h>

static const char *getHildonTranslation(const char *string)
{
     setlocale (LC_ALL, "");
     const char *translation = ::dgettext("hildon-libs", string);
     if (qstrcmp(string, translation) == 0)
         return 0;
     return translation;
}

const char *hildonDateDayNameShrt = getHildonTranslation("wdgt_va_date_day_name_short");
const char *hildonAMfrmt = getHildonTranslation("wdgt_va_12h_time_am");
const char *hildonPMfrmt = getHildonTranslation("wdgt_va_12h_time_pm");
const char *hildon24frmt = getHildonTranslation("wdgt_va_24h_time");
const char *hildonHHfrmt = getHildonTranslation("wdgt_va_24h_hours");
bool dl_started = false;

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
    this->setWindowFlags(Qt::Window);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);

    this->setWindowTitle(_("cloc_ti_world_clocks"));

    ui->newWorldclock_pushButton->setIcon(QIcon::fromTheme("general_add"));
    ui->newWorldclock_pushButton->setText(_("cloc_me_new_world_clock"));

    FileDelegate *pluginDelegate = new FileDelegate(ui->treeWidget);
    ui->treeWidget->setItemDelegate(pluginDelegate);

    getAMPM();
    loadCurrent();

    QSettings settings("worldclock", "worldclock");
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
    localPMtxt = localPMtxt.remove(QRegExp("(\\,|\\.)"));
    QString localAMtxt = QLocale::system().amText();
    localAMtxt = localAMtxt.remove(QRegExp("(\\,|\\.)"));

    if ( (data.contains(localAMtxt)) || (data.contains(localPMtxt)) )
        return localAMtxt;
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
	// portrait
        ui->treeWidget->header()->resizeSection(0,116+len);
        ui->treeWidget->header()->resizeSection(1,210-len);
        ui->treeWidget->header()->resizeSection(2,100);
    } else {
	// landscape
        ui->treeWidget->header()->resizeSection(0,116+len);
        ui->treeWidget->header()->resizeSection(1,424-len);
        ui->treeWidget->header()->resizeSection(2,206);
    }
    ui->treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

void World::getAMPM()
{
       // get the current time format
       GConfItem *HH24 = new GConfItem("/apps/clock/time-format");
       HH24true = HH24->value().toBool();
       delete HH24;
}

void World::on_newWorldclock_pushButton_clicked()
{
    Dialog3* hw = new Dialog3(this);
    hw->exec();
    if ( hw->selected != "" )
    {
	// load current worlclock numbers from config file
        QSettings settings("worldclock", "worldclock");
        QStringList listado = settings.value("Cities",QStringList()).toStringList();
	if ( listado.contains(hw->selected) ) {
            QString notificationDuplicate = "dbus-send --type=method_call --dest=org.freedesktop.Notifications \
                        /org/freedesktop/Notifications org.freedesktop.Notifications.SystemNoteInfoprint \
                        string:\"" + _("cloc_ib_world_clock_same") + "\"";
            QProcess::startDetached(notificationDuplicate);
	}
	else {
             addCity(hw->selected.toInt());
             listado.append(hw->selected);
             settings.setValue("Cities", listado);
             settings.sync();
        }
    }
    delete hw;
}

void World::addCity(int cityId)
{
    QTreeWidgetItem *pepe = new QTreeWidgetItem();

    bool UTCtime = false;
    QString n;
    if (cityId == 999)
    {
	// our own added UTC timezone
	UTCtime = true;
	// UTC is equal to iceland timezone, so use we that info
	cityId = 164;
    }
    Cityinfo* city = cityinfo_from_id(cityId);

    if (UTCtime)
    {
	// restore own UTC timezone id
	cityId = 999;
        n = QString::fromUtf8("GMT") + "  startdesc" + QString::fromUtf8("UTC");
    }
    else
        n = QString::fromUtf8(city->name) + "  startdesc" + QString::fromUtf8(city->country);
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
    {
         int minutes = -offset/60 %60;
         timeoffset = QString("GMT %3:%4").arg(sign+QString::number(-offset/3600)).arg(abs(minutes));
    }

    QDateTime tiempo = QDateTime::currentDateTime();

    tiempo = tiempo.toUTC();
    tiempo = tiempo.addSecs( -offset );

    if (HH24true)
          pepe->setText(0, formatHildonDate(tiempo, hildon24frmt) );
    else
    {
          QString HH = formatHildonDate(tiempo, hildonHHfrmt);
          if ( HH.toInt() > 11 )
                pepe->setText(0, formatHildonDate(tiempo, hildonPMfrmt) );
          else
                pepe->setText(0, formatHildonDate(tiempo, hildonAMfrmt) );
    }
    pepe->setWhatsThis(0, "time");
    QString CurrentdayNameShort = formatHildonDate(tiempo, hildonDateDayNameShrt);
    // also remove trailing comma/point
    QStringList sl = CurrentdayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
    CurrentdayNameShort = sl.at(0);
    QString LocalDateShort = tiempo.date().toString(Qt::SystemLocaleShortDate);

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
    // get the current time format
    getAMPM();
    // get the current home city
    GConfItem *homeCityCode = new GConfItem("/apps/clock/home-location");
    QString timeoffset;
    ret = time_get_timezone(&current_tz[0], 32);
    if(ret>0 && ret < 32)
    {
      Cityinfo** cities = cityinfo_get_all();
      Cityinfo** cities_iter = cities;
      bool current_found = false;
      while(*cities_iter && !current_found)
      {
        int cityId = cityinfo_get_id(*cities_iter);

        if(homeCityCode->value().toInt()==cityId)
        {
          current_found = true;

          QTreeWidgetItem *pepe = new QTreeWidgetItem();

          QDateTime tiempo = QDateTime::currentDateTime();
          if (HH24true)
              pepe->setText(0, formatHildonDate(tiempo, hildon24frmt) );
          else
          {
              QString HH = formatHildonDate(tiempo, hildonHHfrmt);
              if ( HH.toInt() > 11 )
                   pepe->setText(0, formatHildonDate(tiempo, hildonPMfrmt) );
              else
                   pepe->setText(0, formatHildonDate(tiempo, hildonAMfrmt) );
          }
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
	  {
              int minutes = -offset/60 %60;
              timeoffset = QString("GMT %3:%4").arg(sign+QString::number(-offset/3600)).arg(abs(minutes));
	  }

          QDate fecha = QDate::currentDate();
          QDateTime date_time = QDateTime::currentDateTime();
          /* Local time */
          QString CurrentdayNameShort = formatHildonDate(date_time, hildonDateDayNameShrt);

          QStringList sl = CurrentdayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
          CurrentdayNameShort = sl.at(0);
          QString LocalDateShort = fecha.toString(Qt::SystemLocaleShortDate);

          pepe->setText(2, CurrentdayNameShort + " "
                        + LocalDateShort
                        + "  startdesc" + timeoffset );

          pepe->setWhatsThis(2, "world-date");
          QString tr = timeoffset;
          tr.remove("GMT ");
          curTime = tr.toInt();

          if ( ui->treeWidget->topLevelItemCount() == 0 )
	          ui->treeWidget->addTopLevelItem(pepe);
          else
          {
                  ui->treeWidget->takeTopLevelItem (0);
                  ui->treeWidget->insertTopLevelItem (0,pepe);
          }

        }
        ++cities_iter;
      }
      cityinfo_free_all(cities);
    }

    delete homeCityCode;

    // local GMT for the mainwindow
    line1 = timeoffset;

    orientationChanged();

}

void World::on_treeWidget_itemActivated(QTreeWidgetItem*)
{
	// Open date/time settings window upon short press of local time
	if (  ui->treeWidget->currentItem()->text(1).contains(_("cloc_fi_local_time")) && not dl_started )
	{
		dl_started = true;

		MainWindow * w = dynamic_cast <MainWindow*> (parent());
		if ( w )
			w->openplugin("libcpdatetime");
                // refresh alarm
                loadCurrent();
                orientationChanged();
    		ui->treeWidget->setCurrentItem(0);
		dl_started = false;
	}
	else
	{
		if (!dl_started)
		{
			// show city details
    			dl_started = true;
			QString CityCountry = ui->treeWidget->currentItem()->text(1).replace("  startdesc",", ");
			QString ExtGMT = ui->treeWidget->currentItem()->text(2).replace("  startdesc","|");
			ExtGMT = ExtGMT.section( "|", 1, 1 );
			QString LocGMT = line1;
			QString SelCityCode = ui->treeWidget->currentItem()->statusTip(0);
			int OffsetSecs = ui->treeWidget->currentItem()->statusTip(1).toInt();
			CityDetail * c_details = new CityDetail(this,CityCountry,SelCityCode,ExtGMT,LocGMT,HH24true,OffsetSecs);
			c_details->show();
			connect(c_details, SIGNAL(destroyed()), this, SLOT(onChildClosed()));
			this->hide();
		}
	}
}

void World::on_treeWidget_customContextMenuRequested(QPoint pos)
{
    if (! ui->treeWidget->currentItem()->text(1).contains(_("cloc_fi_local_time")) )
    {
	    intl("rtcom-call-ui");
	    QMenu *contextMenu = new QMenu(this);
	    contextMenu->addAction(QIcon(), _("call_me_context_menu_delete"), this, SLOT(removeSel()));
	    intl("osso-clock");
	    contextMenu->exec(mapToGlobal(pos));
    }
}


void World::removeSel()
{
    // remove currentItem from list
    ui->treeWidget->takeTopLevelItem( ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->currentItem()) );
    QStringList listado;
    if ( ui->treeWidget->topLevelItemCount() > 1 )
    {
        for (int i=0; i<ui->treeWidget->topLevelItemCount()-1; ++i)
            listado.append( ui->treeWidget->topLevelItem(i+1)->statusTip(0) );
    }
    // save to config file
    QSettings settings("worldclock", "worldclock");
    settings.setValue("Cities", listado);
    settings.sync();

}

void World::updateClocks()
{
    QDateTime tiempo = QDateTime::currentDateTime();
    if (HH24true)
          ui->treeWidget->topLevelItem(0)->setText(0, formatHildonDate(tiempo, hildon24frmt) );
    else
    {
        QString HH = formatHildonDate(tiempo, hildonHHfrmt);
        if ( HH.toInt() > 11 )
             ui->treeWidget->topLevelItem(0)->setText(0, formatHildonDate(tiempo, hildonPMfrmt) );
        else
             ui->treeWidget->topLevelItem(0)->setText(0, formatHildonDate(tiempo, hildonAMfrmt) );
    }

    if ( ui->treeWidget->topLevelItemCount() > 1 )
    {
        for (int i=0; i<ui->treeWidget->topLevelItemCount()-1; ++i)
        {
            int off = ui->treeWidget->topLevelItem(i+1)->statusTip(1).toInt();
            tiempo = QDateTime::currentDateTime();
            tiempo = tiempo.toUTC();
            tiempo = tiempo.addSecs( -off );
            if (HH24true)
               ui->treeWidget->topLevelItem(i+1)->setText(0, formatHildonDate(tiempo, hildon24frmt) );
            else
            {
               QString HH = formatHildonDate(tiempo, hildonHHfrmt);
               if ( HH.toInt() > 11 )
                  ui->treeWidget->topLevelItem(i+1)->setText(0, formatHildonDate(tiempo, hildonPMfrmt) );
               else
                  ui->treeWidget->topLevelItem(i+1)->setText(0, formatHildonDate(tiempo, hildonAMfrmt) );
            }
        }

    }
}

void World::onChildClosed()
{
    this->show();
    QSettings settings("worldclock", "worldclock");
    QStringList citylist = settings.value("Cities",QStringList()).toStringList();
    // remove from internal list if no longer in config file
    if (!citylist.contains(ui->treeWidget->currentItem()->statusTip(0)))
    	ui->treeWidget->takeTopLevelItem( ui->treeWidget->indexOfTopLevelItem(ui->treeWidget->currentItem()) );
    ui->treeWidget->setCurrentItem(0);
    dl_started = false;
}
