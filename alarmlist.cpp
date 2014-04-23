#include "alarmlist.h"
#include "ui_alarmlist.h"
#include "filedelegate.h"
#include "newalarm.h"
#include "gconfitem.h"
#include "osso-intl.h"
#include <QDebug>
#include <QMaemo5Style>
#include <QSettings>
#include <QDesktopWidget>
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

const char *hildonDayOfWeek = getHildonTranslation("wdgt_va_week");
const char *hildonAMformat = getHildonTranslation("wdgt_va_12h_time_am");
const char *hildonPMformat = getHildonTranslation("wdgt_va_12h_time_pm");
const char *hildon24format = getHildonTranslation("wdgt_va_24h_time");
const char *hildonHHformat = getHildonTranslation("wdgt_va_24h_hours");


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

AlarmList::AlarmList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlarmList)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);

    this->setWindowTitle(_("cloc_ti_alarms"));

    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setWindowFlags(Qt::Window);

    ui->treeWidget->hideColumn(4);

    loadAlarms();

    FileDelegate *pluginDelegate = new FileDelegate(ui->treeWidget);
    ui->treeWidget->setItemDelegate(pluginDelegate);

    ui->newAlarm->setIcon(QIcon::fromTheme("general_add"));
    ui->newAlarm->setText(_("clock_ti_new_alarm"));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

    // refresh the current alarmlist as soon as an alarm event occurs
    QDBusConnection::systemBus().connect(QString(),
                                 "/com/nokia/alarmd",
				 "com.nokia.alarmd",
				 "queue_status_ind",
	                         this, SLOT(loadAlarms()));

}

AlarmList::~AlarmList()
{
    delete ui;
}

QString AlarmList::longdate(QString data)
{

    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();
    if ( (data.contains(localAMtxt)) || (data.contains(localPMtxt)) )
        return localAMtxt;
    else
        return "no";

}

void AlarmList::orientationChanged()
{
    int len = 0;
    if ( ui->treeWidget->topLevelItemCount() > 0 )
    {
        if ( longdate(ui->treeWidget->topLevelItem(0)->text(1)) != "no" )
            len = 26;
    }

    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height())
    {
        ui->treeWidget->header()->resizeSection(0,54);
        ui->treeWidget->header()->resizeSection(1,110+len);
        ui->treeWidget->header()->resizeSection(2,138-len);
        ui->treeWidget->header()->resizeSection(3,130);
    } else {
        ui->treeWidget->header()->resizeSection(0,54);
        ui->treeWidget->header()->resizeSection(1,110+len);
        ui->treeWidget->header()->resizeSection(2,458-len);
        ui->treeWidget->header()->resizeSection(3,130);
    }
    ui->treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

void AlarmList::loadAlarms()
{
    ui->treeWidget->clear();

    int activeAlarms = 0;
    // bool exactDate = false;
    //find the date separator symbol
    QString dateSep = QDate::currentDate().toString(Qt::SystemLocaleShortDate).remove(QRegExp("\\d+|\\s+")).at(0);

    cookie_t *list, *iter; // cookie_t - alarm cookies from GTK

    // set number of hours to search alarms
    int timeFrame = 24*7;

    // get list of alarms
    list = alarmd_event_query(0, 0, 0, 0, "worldclock_alarmd_id");

    if (list[0] != (cookie_t) 0) {
	    ui->label->hide();
    	    ui->treeWidget->show();
        // iterate through alarm list
        for (iter = list; *iter != (cookie_t) 0; iter++)
        {
            // get alarm event by it's cookie
            alarm_event_t * aevent = alarmd_event_get (*iter);
            // check alarm recurrence
            //int recur1 = alarm_event_is_recurring (aevent);
            // get time of alarm action
            time_t ttime = alarm_event_get_trigger(aevent);

            // convert GTK's ttime to Qt's DateTime
            QDateTime qdtm = QDateTime::fromTime_t(ttime);
            // get time part from DateTime variable
            QTime qtm = qdtm.time();
            QTime qtm_sched(aevent->alarm_tm.tm_hour,aevent->alarm_tm.tm_min, 0);   
            QDateTime qdtm_sched = QDateTime::fromTime_t(ttime);
	    // set time without snooze:
            qdtm_sched.setTime(qtm_sched);
            // let's count day to current alarm triggering
            // int days = 0;
            QDateTime currDate;
            if (timeFrame > 24) // if we need to look forward for more then one day
            {
                //  get current date from system
                currDate = QDateTime::currentDateTime();
                // calculate seconds from current time to alarm's trigger
                // int secs = currDate.secsTo(qdtm);
                // get full days until alarm
                // days = secs / (60 * 60 * 24);
            }
            // here we will form alarm's string
            QString sTime;
            // using locale settings to convert time into string
            sTime = qtm.toString(Qt::DefaultLocaleShortDate);

            QString aTitle;
            const char * cTitle = alarm_event_get_message(aevent);
            aTitle = QString::fromUtf8(cTitle);
            if (aTitle.isEmpty())
            {
                cTitle = alarm_event_get_title(aevent);
                aTitle = QString::fromUtf8(cTitle);
            }

            long cook1 = aevent->cookie;

            QTreeWidgetItem *pepe = new QTreeWidgetItem();

            // get the current time format
            GConfItem *HH24 = new GConfItem("/apps/clock/time-format");
            bool HH24true = HH24->value().toBool();

	    // if non repeating do not look at trigger time (incl snooze)
	    // alarm_time != -1 should mean alarm on exact date/time
            if ( aevent->alarm_tm.tm_hour == -1 || aevent->alarm_time != -1) {
                // as we now have an exact epoch stamp from the alarm without snooze, let's use that one
                time_t altime = aevent->alarm_time;
                QDateTime aldtm = QDateTime::fromTime_t(altime);
                if (HH24true)
                {
                    pepe->setText(1, formatHildonDate(aldtm, hildon24format) );
                    pepe->setText(5, formatHildonDate(qdtm, hildon24format) );
                }
                else
                {
                    QString HH = formatHildonDate(qdtm, hildonHHformat);
                    if ( HH.toInt() > 11 )
                    {
                         pepe->setText(1, formatHildonDate(aldtm, hildonPMformat) );
                         pepe->setText(5, formatHildonDate(qdtm, hildonPMformat) );
                    }
                    else
                    {
                         pepe->setText(1, formatHildonDate(aldtm, hildonAMformat) );
                         pepe->setText(5, formatHildonDate(qdtm, hildonAMformat) );
                    }
                }
            }
            else {
                if (HH24true)
                {
                    pepe->setText(1, formatHildonDate(qdtm_sched, hildon24format) );
                    pepe->setText(5, formatHildonDate(qdtm, hildon24format) );
                }
                else
                {
                    QString HH = formatHildonDate(qdtm, hildonHHformat);
                    if ( HH.toInt() > 11 )
                    {
                         pepe->setText(1, formatHildonDate(qdtm_sched, hildonPMformat) );
                         pepe->setText(5, formatHildonDate(qdtm, hildonPMformat) );
                    }
                    else
                    {
                         pepe->setText(1, formatHildonDate(qdtm_sched, hildonAMformat) );
                         pepe->setText(5, formatHildonDate(qdtm, hildonAMformat) );
                    }
                }
            }
            pepe->setWhatsThis(1, "time");

            pepe->setText(2, aTitle );
            pepe->setWhatsThis(2, "name");

            pepe->setText(4, QString::number(ttime));

            unsigned f = aevent->flags;

            if ( f==136240 || f==131072 || f==132648 || f==131624 || f==552 || f==1576 || \
                 f==131632 || f==560 || f==132656 || f==36323632 || f==132656 || f==1584 )
            {
                pepe->setText(0,"active");
                pepe->setStatusTip(1, "active");
                pepe->setStatusTip(2, "active");
                ++activeAlarms;
            }
            else
            {
                pepe->setText(0,"inactive");
                pepe->setStatusTip(1, "inactive");
                pepe->setStatusTip(2, "inactive");
            }

            if ( aevent->recurrence_tab )
            {
                uint32_t dias = aevent->recurrence_tab->mask_wday;

                if ( dias == ALARM_RECUR_WDAY_ALL )
                    pepe->setText(3, "8");
                else if ( dias == ALARM_RECUR_WDAY_DONTCARE )
                    pepe->setText(3,"0");
                else
                {
                    QStringList days;
                    int sunday = 0;
                    if ( dias >= ALARM_RECUR_WDAY_SAT )
                    {
                        days.append("6");
                        dias = dias - ALARM_RECUR_WDAY_SAT;
                    }
                    if ( dias >= ALARM_RECUR_WDAY_FRI )
                    {
                        days.append("5");
                        dias = dias - ALARM_RECUR_WDAY_FRI;
                    }
                    if ( dias >= ALARM_RECUR_WDAY_THU )
                    {
                        days.append("4");
                        dias = dias - ALARM_RECUR_WDAY_THU;
                    }

                    if ( dias >= ALARM_RECUR_WDAY_WED )
                    {
                        days.append("3");
                        dias = dias - ALARM_RECUR_WDAY_WED;
                    }

                    if ( dias >= ALARM_RECUR_WDAY_TUE )
                    {
                        days.append("2");
                        dias = dias - ALARM_RECUR_WDAY_TUE;
                    }

                    if ( dias >= ALARM_RECUR_WDAY_MON )
                    {
                        days.append("1");
                        dias = dias - ALARM_RECUR_WDAY_MON;
                    }

                    if ( dias == ALARM_RECUR_WDAY_SUN )
                    {
                        sunday = 1;
                        dias = dias - ALARM_RECUR_WDAY_SUN;
                    }

                    QStringList days2;
                    for (int j=0; j<days.count(); ++j)
                        days2.append( days.at( days.count()-1-j ) );

                    if ( sunday == 1 )
                        days2.append("7");

                    pepe->setText(3, days2.join(", "));

                }
            }
            else
            {
                if (aevent->alarm_time != -1) 
		{
			// looks like we have a time incl date alarm
			// exactDate = true;
			QDateTime timestamp;
			timestamp.setTime_t(aevent->alarm_time);
			QString LocalDateShort = timestamp.date().toString(Qt::SystemLocaleShortDate).remove(QRegExp("\\W$"));
			pepe->setText(3, LocalDateShort + timestamp.toString(dateSep+"yyyy"));
		}
		else
			pepe->setText(3, "0");
            }
            pepe->setWhatsThis(3, "days");
            pepe->setStatusTip(0, QString::number(cook1) );

            ui->treeWidget->addTopLevelItem(pepe);

            // free alarm event structure
            alarm_event_delete(aevent);
        }
    }
    else
    {
        QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
        ui->label->setStyleSheet(QString("color: rgb(%1, %2, %3);")
                                         .arg(secondaryColor.red())
                                         .arg(secondaryColor.green())
                                         .arg(secondaryColor.blue()));
        QFont fontNoAlarm;
        fontNoAlarm.setPointSize(24);
	ui->label->setFont(fontNoAlarm);
	ui->label->setAlignment(Qt::AlignCenter);
	ui->label->setText(_("cloc_ti_start_no"));
        ui->treeWidget->hide();
        ui->label->show();
    }
    free(list);

    ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(4, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    orientationChanged();

    if ( (ui->treeWidget->topLevelItemCount()>0) && (activeAlarms>0) )
    {
        line1 = _("cloc_ti_next") + " " + ui->treeWidget->topLevelItem(0)->text(5);
        line2 = "";
        QDateTime qdtm = QDateTime::fromTime_t( ui->treeWidget->topLevelItem(0)->text(4).toInt() );
        QDateTime currDate;
        currDate = QDateTime::currentDateTime();
        int ds = currDate.daysTo(qdtm);
        if ( ds == 1 )
            line2 = _("cloc_ti_start_tomorrow");
        else if ( ds > 1 && ds < 8 )
        {
            QString tmp = ui->treeWidget->topLevelItem(0)->text(3);
            int j = tmp.indexOf(",");
            if ( j>0 )
                tmp.remove(j, tmp.length()-j);
            line2 = _("cloc_ti_start_day");
            line2.replace("%s", formatHildonDate(qdtm, hildonDayOfWeek));
        }
        else if ( ds > 7 )
	{
			// show date
            line2 = ui->treeWidget->topLevelItem(0)->text(3);
	}

    }
    else
    {
        line1 = _("cloc_ti_start_no");
        line2 = "";
    }


}

void AlarmList::on_newAlarm_clicked()
{
    NewAlarm *al = new NewAlarm(this,false,"","","0",true,0);
    al->exec();
    delete al;
    loadAlarms();
    ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(4, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->clearSelection();

}


void AlarmList::on_treeWidget_itemActivated(QTreeWidgetItem* item, int column)
{
    bool checked = false;
    if ( item->text(0) == "active" )
        checked = true;

    /* column=0: pressed on clock icon: toggle alarm active/inactive
       column=1: pressed on time
       column=2: pressed on text
       column=3: pressed on repeat field
       text(1): time
       text(2): title
       text(3): date/repeat */
    if ( column != 0 )
    {
        NewAlarm *al = new NewAlarm(this, true, item->text(2),                        //alarmtext
                                        item->text(1), item->text(3).replace(" ",""), //1=time 3=date
                                        checked, item->statusTip(0).toLong() );       //cookie-nbr
        al->exec();
        delete al;
        loadAlarms();
    }
    else
    {
	//get date&time of alarm
	alarm_event_t *eve = 0;
	eve = alarmd_event_get(item->statusTip(0).toLong());
	if ( eve->alarm_time == -1 || eve->alarm_time > QDateTime::currentDateTime().toTime_t())
	{
		// it is no specific date&time alarm, or original alarm date&time are in the future
		// toggle active/inactive
		NewAlarm *al = new NewAlarm(this, true, item->text(2),
						item->text(1), item->text(3),
						!checked, item->statusTip(0).toLong() );
		// remove current alarm
		al->removeAlarm(item->statusTip(0).toLong());
		// add toggled on
		al->addAlarm();
		delete al;
		loadAlarms();
	}
	// cleanup memory
	alarm_event_delete(eve);
    }
    ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(4, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->clearSelection();

}
