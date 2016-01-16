#include "qalarmdialog.h"
#include "qnewalarmdialog.h"
#include "osso-intl.h"

#include <QMaemo5Style>
#include <QDesktopWidget>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QDBusConnection>
#include <QTimer>

#include "utils.h"

enum {
    AlarmEnabledRole = Qt::UserRole + 1,
    AlarmCookieRole,
    AlarmWdayRole,
    AlarmDateTimeRole
};

QAlarmDialog::QAlarmDialog(QWidget *parent) :
    QDialog(parent),
    label(new QLabel),
    button(new QPushButton()),
    view(new QTreeView()),
    model(new QStandardItemModel(0, 4, view))
{
    setWindowTitle(_("cloc_ti_alarms"));
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setWindowFlags(Qt::Window);

    /* model/view */
    model->setSortRole(AlarmEnabledRole);

    view->setModel(model);
    view->setColumnWidth(0, 64);
    view->setColumnWidth(1, 96);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setIconSize(QSize(48, 48));
    view->setIndentation(0);
    view->setUniformRowHeights(true);
    view->setHeaderHidden(true);

    /* button */
    button->setIconSize(QSize(48, 48));
    button->setIcon(QIcon::fromTheme("general_add"));
    button->setText(_("clock_ti_new_alarm"));

    /* label */
    QPalette pal(palette());
    pal.setColor(QPalette::WindowText,
                 QMaemo5Style::standardColor("SecondaryTextColor"));
    label->setPalette(pal);
    label->setFont(QMaemo5Style::standardFont("XX-LargeSystemFont"));
    label->setAlignment(Qt::AlignCenter);
    label->setText(_("cloc_ti_start_no"));

    /* put them all together */
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(button);
    layout->addWidget(label);
    layout->addWidget(view);

    /* if someone has connected to our changed signals to have the real data */
    QTimer::singleShot(0, this, SLOT(addAlarms()));

    /* refresh the current alarmlist as soon as an alarm event occurs */
    QDBusConnection::systemBus().connect(QString(), "/com/nokia/alarmd",
                                         "com.nokia.alarmd", "queue_status_ind",
                                         this, SLOT(addAlarms()));
    connect(view, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(viewClicked(const QModelIndex &)));

    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

QStandardItem *QAlarmDialog::alarmCheckboxItem(cookie_t cookie,
                                             const alarm_event_t *ae)
{
    QStandardItem *item = new QStandardItem();

    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setEditable(false);

    /* enable/disable check box */
    if (!(ae->flags & ALARM_EVENT_DISABLED))
    {
        item->setIcon(QIcon::fromTheme("clock_alarm_on").pixmap(48, 48));
        item->setData(1, AlarmEnabledRole);
    }
    else
    {
        item->setIcon(QIcon::fromTheme("clock_alarm_off").pixmap(48, 48));
        item->setData(0, AlarmEnabledRole);
    }

    item->setData((qlonglong)cookie, AlarmCookieRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmTimeItem(const alarm_event_t *ae)
{
    QStandardItem *item = new QStandardItem();
    time_t tick = ae->snooze_total;

    if (!tick)
        tick = ae->trigger;

    item->setText(formatDateTime(tick, Time));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setFont(QMaemo5Style::standardFont("LargeSystemFont"));

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    /* needed so sorting to work */
    item->setData(item->text());
    item->setData((int)tick, AlarmDateTimeRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmTitleItem(const alarm_event_t *ae)
{
    const char *s = alarm_event_get_message(ae);

    if(!s)
        s = alarm_event_get_title(ae);

    QStandardItem *item = new QStandardItem(s);

    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmDaysItem(const alarm_event_t *ae)
{
    QStandardItem *item = new QStandardItem();
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    uint32_t wday = 0;
    QString days;

    if (ae->recurrence_tab)
        wday = ae->recurrence_tab->mask_wday;

    QStringList l = daysFromWday(wday);

    if (l.count() > 3)
        days = QStringList(l.mid(0, 3)).join(", ") + "\n" +
                QStringList(l.mid(3)).join(", ");
    else
        days = l.join(", ");

    item->setData(wday, AlarmWdayRole);
    item->setText(days);
    item->setFont(QMaemo5Style::standardFont("SmallSystemFont"));

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    return item;
}

/* return alarm trigger time */
time_t QAlarmDialog::addAlarm(cookie_t cookie)
{
    QList<QStandardItem *> row;
    alarm_event_t *ae = alarmd_event_get(cookie);
    time_t tick = (ae->flags & ALARM_EVENT_DISABLED) ? -1 : ae->trigger;

    row << alarmCheckboxItem(cookie, ae)
        << alarmTimeItem(ae)
        << alarmTitleItem(ae)
        << alarmDaysItem(ae);

    alarm_event_delete(ae);

    model->appendRow(row);

    return tick;
}

void QAlarmDialog::addAlarms()
{
    cookie_t *list, *iter;
    time_t next = -1;
    QString nextAlarmDate;
    QString nextAlarmDay;

    model->removeRows(0, model->rowCount());
    /* get them */
    list = alarmd_event_query(0, 0, 0, 0, "worldclock_alarmd_id");
    if (*list)
    {
      label->hide();
      view->show();
    }
    else
    {
        view->hide();
        label->show();
    }

    for (iter = list; *iter != (cookie_t) 0; iter ++)
    {
        time_t t = addAlarm(*iter);

        if (t != -1 && (t < next || next == -1))
            next = t;
    }

    free(list);

    /* need to sort in reverse order :) */
    model->sort(1);
    model->sort(0, Qt::DescendingOrder);

    if (next != -1)
    {
        nextAlarmDate = _("cloc_ti_next") + " " + formatDateTime(next, Time);
        int days =
            QDateTime::currentDateTime().daysTo(QDateTime::fromTime_t(next));

        if (days == 1)
            nextAlarmDay = _("cloc_ti_start_tomorrow");
        else if (days < 8)
        {
            nextAlarmDay = QString(_("cloc_ti_start_day")).
                    replace("%s", formatDateTime(next, DayOfWeek));
        }
        else
            nextAlarmDay = formatDateTime(next, Date);
    }
    else
        nextAlarmDate = _("cloc_ti_start_no");
#if 0
    ui->alarmListView->clear();

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
            ui->alarmListView->show();
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
            if ( aevent->alarm_tm.tm_hour == -1 && aevent->alarm_time != -1) {
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
                if ( aevent->recurrence_tab )
                {
                    time_t org_time = aevent->snooze_total;
                    if ( org_time > 0 ) {
                        // snooze is set. If not it is 0
                        QDateTime qt_dtm = QDateTime::fromTime_t(org_time);
                        // get time part
                        QTime qt_time = qt_dtm.time();
                        // and make it the time displayed in the list
                        qdtm_sched.setTime(qt_time);
                     }
                     else
                         qdtm_sched.setTime(qtm); // display trigger time
                }
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

            ui->alarmListView->addTopLevelItem(pepe);

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
        ui->alarmListView->hide();
        ui->label->show();
    }
    free(list);

    ui->alarmListView->sortByColumn(1, Qt::AscendingOrder);
    ui->alarmListView->sortByColumn(4, Qt::AscendingOrder);
    ui->alarmListView->sortByColumn(0, Qt::AscendingOrder);
    orientationChanged();

    if ( (ui->alarmListView->topLevelItemCount()>0) && (activeAlarms>0) )
    {
            line1 = _("cloc_ti_next") + " " + ui->alarmListView->topLevelItem(0)->text(5);
            line2 = "";
            QDateTime qdtm = QDateTime::fromTime_t( ui->alarmListView->topLevelItem(0)->text(4).toInt() );
            QDateTime currDate;
            currDate = QDateTime::currentDateTime();
            int ds = currDate.daysTo(qdtm);
            if ( ds == 1 )
                line2 = _("cloc_ti_start_tomorrow");
            else if ( ds > 1 && ds < 8 )
            {
                QString tmp = ui->alarmListView->topLevelItem(0)->text(3);
                int j = tmp.indexOf(",");
                if ( j>0 )
                    tmp.remove(j, tmp.length()-j);
                line2 = _("cloc_ti_start_day");
                line2.replace("%s", formatHildonDate(qdtm, hildonDayOfWeek));
            }
            else if ( ds > 7 )
            {
                // show date
                line2 = ui->alarmListView->topLevelItem(0)->text(3);
            }

    }
    else
    {
        line1 = _("cloc_ti_start_no");
        line2 = "";
    }

#endif

    emit nextAlarmChanged(QStringList () << nextAlarmDate << nextAlarmDay);
}

void QAlarmDialog::buttonClicked()
{
    QNewAlarmDialog(this,false,"",QTime(),0,true,0).exec();

    addAlarms();
}

void QAlarmDialog::viewClicked(const QModelIndex &modelIndex)
{
    int row = modelIndex.row();
    bool disabled = model->item(row, 0)->data(AlarmEnabledRole).toBool();
    uint seconds = model->item(row, 1)->data(AlarmDateTimeRole).toInt();
    QDateTime dt = QDateTime::fromTime_t(seconds);
    uint32_t wday = model->item(row, 3)->data(AlarmWdayRole).toUInt();
    QString text = model->item(row, 2)->text();
    cookie_t cookie = model->item(row, 0)->data(AlarmCookieRole).toLongLong();

    if (modelIndex.column() == 0)
    {
        /* toggle enabled state */
        alarm_event_t *ae = alarmd_event_get(cookie);
        ae->flags &= ~ALARM_EVENT_DISABLED;
        ae->flags |= disabled ? ALARM_EVENT_DISABLED : 0;
        alarmd_event_update(ae);

        if (!disabled)
            showAlarmTimeBanner(ae->trigger);

        alarm_event_delete(ae);
    }
    else
        QNewAlarmDialog(this, true, text, dt.time(), wday, disabled, cookie).exec();

    addAlarms();
}
