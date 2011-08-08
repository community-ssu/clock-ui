#include "qmaemo5rotator.h"
#include "alarmlist.h"
#include "ui_alarmlist.h"
#include "filedelegate.h"
#include "newalarm.h"
#include "osso-intl.h"
#include <QDebug>
#include <QSettings>

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

    ui->pushButton->setIcon(QIcon::fromTheme("general_add"));
    ui->pushButton->setText(_("clock_ti_new_alarm"));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

}

AlarmList::~AlarmList()
{
    delete ui;
}

QString AlarmList::longdate(QString data)
{

    if ( (data.contains("am")) || (data.contains("pm")) )
        return "am";
    else if ( (data.contains("a.m.")) || (data.contains("p.m.")) )
        return "a.m.";
    else if ( (data.contains("AM")) || (data.contains("PM")) )
        return "AM";
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

    cookie_t *list, *iter; // cookie_t - alarm cookies from GTK
    time_t now; // time_t structure from GTK

    // set number of hours to search alarms
    int timeFrame = 24*7;

    now = time(NULL);
    // get list of alarms
    list = alarmd_event_query(0, 0, 0, 0, "worldclock_alarmd_id");

    if (list[0] != (cookie_t) 0) {
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
            // let's count day to current alarm triggering
            int days = 0;
            QDateTime currDate;
            if (timeFrame > 24) // if we need to look forward for more then one day
            {
                //  get current date from system
                currDate = QDateTime::currentDateTime();
                // calculate seconds from current time to alarm's trigger
                int secs = currDate.secsTo(qdtm);
                // get full days until alarm
                days = secs / (60 * 60 * 24);
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

            pepe->setText(1, QLocale::system().toString( qtm, QLocale::ShortFormat) );
            pepe->setWhatsThis(1, "time");

            pepe->setText(2, aTitle );
            pepe->setWhatsThis(2, "name");

            pepe->setText(4, QString::number(ttime));

            //qDebug() << "DAYS FOR ALARM: " << cook1 << dias << fl1;

            unsigned f = aevent->flags;
            if ( f==136240 || f==131072 || f==132648 || f==131624 || f==552 || f==1576 )
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
                        //days.append(QDate::shortDayName(7));
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
                pepe->setText(3, "0");
            }
            pepe->setWhatsThis(3, "days");
            pepe->setStatusTip(0, QString::number(cook1) );

            ui->treeWidget->addTopLevelItem(pepe);


            // free alarm event structure
            alarm_event_delete(aevent);
        }
    }
    free(list);

    ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(4, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    orientationChanged();

    if ( (ui->treeWidget->topLevelItemCount()>0) && (activeAlarms>0) )
    {
        line1 = _("cloc_ti_next") + " " + ui->treeWidget->topLevelItem(0)->text(1);
        line2 = "";
        QDateTime qdtm = QDateTime::fromTime_t( ui->treeWidget->topLevelItem(0)->text(4).toInt() );
        QDateTime currDate;
        currDate = QDateTime::currentDateTime();
        int ds = currDate.daysTo(qdtm);
        //qDebug() << "DIAS PARA: " << ds;
        if ( ds == 1 )
            line2 = _("cloc_ti_start_tomorrow");
        else if ( ds > 1 )
        {
            QString tmp = ui->treeWidget->topLevelItem(0)->text(3);
            int j = tmp.indexOf(",");
            if ( j>0 )
                tmp.remove(j, tmp.length()-j);
            line2 = QDate::longDayName(tmp.toInt());
        }

    }
    else
    {
        line1 = _("cloc_ti_start_no");
        line2 = "";
    }


}

void AlarmList::on_pushButton_pressed()
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

    if ( column != 0 )
    {
        NewAlarm *al = new NewAlarm(this, true, item->text(2),
                                        item->text(1), item->text(3),
                                        checked, item->statusTip(0).toLong() );
        al->exec();
        delete al;
        loadAlarms();

    }
    else
    {
        NewAlarm *al = new NewAlarm(this, true, item->text(2),
                                        item->text(1), item->text(3),
                                        !checked, item->statusTip(0).toLong() );

        al->removeAlarm(item->statusTip(0).toLong());
        al->addAlarm();
        delete al;
        loadAlarms();
    }

    ui->treeWidget->sortByColumn(1, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(4, Qt::AscendingOrder);
    ui->treeWidget->sortByColumn(0, Qt::AscendingOrder);
    ui->treeWidget->clearSelection();

}
