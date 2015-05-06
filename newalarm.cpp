#include "newalarm.h"
#include "ui_newalarm.h"
#include "osso-intl.h"
#include "libalarm.h"
#include "dialog.h"
#include "dialog2.h"
#include "dialogdate.h"
#include "gconfitem.h"
#include <QDateTime>
#include "valuebutton.h"
#include "alarmsndpick.h"
#include "utils.h"

/* osso-clock text needed for snooze/stop button translation */
#define TEXTDOMAIN "osso-clock"
#define STOP_BUTTON_KEY "cloc_bd_stop"
#define SNOOZE_BUTTON_KEY "cloc_bd_alarm_notification_snooze"

QString sndFile;
QString dateSep = QDate::currentDate().toString(Qt::SystemLocaleShortDate).remove(QRegExp("\\d+|\\s+")).at(0);
bool dateChoosen = false;
bool fromEdit = false;

NewAlarm::NewAlarm(QWidget *parent, bool edit, QString Aname,
                   QString Atime, uint32_t wday, bool Acheck, long Acook, bool show):

    QDialog(parent),
    ui(new Ui::NewAlarm)
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    ui->setupUi(this);

    // get the current default alarm tone
    GConfItem *item = new GConfItem("/apps/clock/alarm-tone");

    sndFile = item->value().toString();
    ui->alsound_pushButton->setEnabled(false);  // <- remove once custom sounds work!

    if (sndFile == "/usr/share/sounds/ui-clock_alarm_default.aac")
        ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone1"));
    else if (sndFile == "/usr/share/sounds/ui-clock_alarm_2.aac")
        ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone2"));
    else if (sndFile == "/usr/share/sounds/ui-clock_alarm_3.aac")
        ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone3"));
    else
        ui->alsound_pushButton->setValueText(QFileInfo(sndFile).baseName());

    isEditing = edit;
    deleted = 1;

    realcookie = Acook;
    time = Atime;
    name = Aname;
    this->wday = wday;
    enabled = Acheck;
    showed = show;

    if ( edit )
    {
        this->setWindowTitle(_("cloc_ti_edit_alarm_title"));
        // we need to find it's current alarm tone
        alarm_event_t *eve = 0;
        eve = alarmd_event_get(realcookie);
        QString aSnd;

        aSnd = QString::fromUtf8(eve->sound);

        if (!aSnd.isEmpty()) // a tone is set
        {
            if ( aSnd == "/usr/share/sounds/ui-clock_alarm_default.aac" )
                ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone1"));
            else if ( aSnd == "/usr/share/sounds/ui-clock_alarm_2.aac" )
                ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone2"));
            else if ( aSnd == "/usr/share/sounds/ui-clock_alarm_3.aac" )
                ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone3"));
            else
            {
                sndFile = aSnd;
                ui->alsound_pushButton->setValueText( QFileInfo(aSnd.remove(QRegExp("\\.ogg$|\\.mp3$|\\.aac$" ))).fileName() );
                aSnd = sndFile;
            }
        }
        // cleanup memory
        alarm_event_delete(eve);
        sndFile = aSnd;
    }
    else
        this->setWindowTitle(_("clock_ti_new_alarm"));

    ui->lineEdit->setPlaceholderText(_("cloc_va_placeholder_title"));
    ui->time_pushButton->setStatusTip(_("cloc_fi_time"));
    ui->date_pushButton->setStatusTip(_("dati_fi_pr_date"));
    ui->repeat_pushButton->setStatusTip(_("cloc_fi_repeat"));
    ui->repeat_pushButton->setValueText("0");
    ui->alsound_pushButton->setStatusTip(_("cloc_ti_alarm_notification_title"));
    ui->repeat_pushButton->setWhatsThis("date");
    ui->checkBox->setText(_("cloc_fi_active"));

    intl("osso-connectivity-ui");
    ui->buttonBox->button(QDialogButtonBox::Retry)->setText(_("conn_bd_devices_delete"));
    ui->buttonBox->button(QDialogButtonBox::Apply)->setText(_("conn_bd_receive_ok"));
    ui->buttonBox_2->button(QDialogButtonBox::Retry)->setText(_("conn_bd_devices_delete"));
    ui->buttonBox_2->button(QDialogButtonBox::Apply)->setText(_("conn_bd_receive_ok"));
    intl("osso-clock");
    ui->buttonBox->button(QDialogButtonBox::Retry)->setVisible(edit);
    ui->buttonBox_2->button(QDialogButtonBox::Retry)->setVisible(edit);

    // set alarmname
    if (!name.isEmpty())
        ui->lineEdit->setText(name);

    // set time
    if (!time.isEmpty())
        ui->time_pushButton->setValueText(time);
    else
        /* FIXME? - stock puts "09:00" here */
        ui->time_pushButton->setValueText(
                 formatDateTime(QDateTime::currentDateTime().toTime_t(), Time));

    time_t nextDate;
    if (realcookie && !wday)
    {
        // get date&time alarm
        alarm_event_t *ae = alarmd_event_get(realcookie);

        if (ae->snooze_total)
            nextDate = ae->snooze_total;
        else if (ae->trigger != -1)
            nextDate = ae->trigger;
        else if (ae->alarm_time != -1)
            nextDate = ae->alarm_time;
        else
            nextDate = QDateTime::currentDateTime().toTime_t();

        if (nextDate < (time_t)QDateTime::currentDateTime().toTime_t())
        {
            /* we can't enable an alarm that is in the past */
            ui->checkBox->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
            ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(false);
        }

        alarm_event_delete(ae);

        ui->date_pushButton->setValueText(daysFromWday(wday).join(","));
    }
    else
        nextDate = QDateTime::currentDateTime().addDays(1).toTime_t();

    ui->date_pushButton->setValueText(formatDateTime(nextDate, FullDateShort));

    if (wday)
    {
        ui->repeat_pushButton->setValueText(daysFromWday(wday).join(","));
        ui->date_pushButton->hide();
    }
    else
        ui->repeat_pushButton->setValueText(_("cloc_va_never"));

    if ( !isEditing )
        ui->checkBox->hide();

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();
}

NewAlarm::~NewAlarm()
{
    delete ui;
}

void NewAlarm::orientationChanged()
{
    int space = 0;
    if ( isEditing )
        space = 76;

    int startspace_portrait = 460;
    int startspace_landscape = 380;
    if ( ui->date_pushButton->isHidden() )
    {
        startspace_portrait = 385;
        startspace_landscape = 305;
    }

    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
	// portrait
        ui->buttonBox->hide();
        ui->buttonBox_2->show();
       	this->setMinimumHeight(startspace_portrait+space+space);
	this->setMaximumHeight(startspace_portrait+space+space);
    } else {
	// landscape
        ui->buttonBox_2->hide();
        ui->buttonBox->show();
        this->setMinimumHeight(startspace_landscape+space);
        this->setMaximumHeight(startspace_landscape+space);
    }

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

}

void NewAlarm::on_time_pushButton_pressed()
{
    QString temp = ui->time_pushButton->valueText();
    int j = temp.indexOf(":");
    temp.remove(j, temp.length()-j);
    int hourval = temp.toInt();
    temp = ui->time_pushButton->valueText();
    j = temp.indexOf(":");
    temp.remove(0, j+1);
    int minuteval = temp.left(2).toInt();
    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();

    bool ampm = false;
    if ( longdate(ui->time_pushButton->valueText()) != "no"  )
        ampm = true;

    bool am = true;
    if (ui->time_pushButton->valueText().contains(localPMtxt) )
        am = false;

    QString dam = localAMtxt;
    QString dpm = localPMtxt;

    Dialog2* hw = new Dialog2(this, hourval, minuteval, ampm, am, dam, dpm);
    int result = hw->exec();

    if (result == QDialog::Accepted)
    {
        QTime tiempo;
        int hora = hw->res1;
        int mins = hw->res2;
        // enable alarm after accepting time
        enabled = true;
        ui->checkBox->setChecked(enabled);
        if ( (longdate(ui->time_pushButton->valueText())!="no") && (!hw->isam) )
        {
            hora = hw->res1+12;
        }
        tiempo.setHMS(hora, mins, 0);

        ui->time_pushButton->setValueText( QLocale::system().toString(tiempo, QLocale::ShortFormat));
        QTime timeNow = QTime::currentTime();
        int hourNow = timeNow.hour();
        int minuteNow = timeNow.minute();
        // The long date string
        if (!dateChoosen && wday)
        {
            // show if the alarm is still today or else tomorrow
            time_t date;
            if ((hora > hourNow) || (hora == hourNow && mins > minuteNow))
                date = QDateTime::currentDateTime().toTime_t();
            else
                date = QDateTime::currentDateTime().addDays(1).toTime_t();

            ui->date_pushButton->setValueText(formatDateTime(date, FullDateShort));
        }
        else
        {
            // enable accept button if new settings in future
            QString dateDisplayed = ui->date_pushButton->valueText();
            QStringList sl = dateDisplayed.remove(QRegExp("(\\,|\\.|^\\w+|\\D+$)")).split(' ', QString::SkipEmptyParts);
            int DayPicked = 1;
            QString MonthPicked;
            int YearPicked = 1900;
            for (int i=0; i<sl.size(); ++i)
            {
               // search through button text (minus weekdayname) for match
               QRegExp rx_year("\\d{4}");
               if ( rx_year.exactMatch(sl.at(i)) )
                    YearPicked = sl.at(i).toInt();
               QRegExp rx_month("\\D+");
               if ( rx_month.exactMatch(sl.at(i)) )
                    MonthPicked = sl.at(i);
               QRegExp rx_day("\\d{1,2}");
               if ( rx_day.exactMatch(sl.at(i)) )
                    DayPicked = sl.at(i).toInt();
            }
            int monthPos=1;
            for (int i=1; i<13; ++i)
            {
                 QString num = QString::number(i);
                 QString monthName = QDateTime::fromString(num,"M").toString(Qt::DefaultLocaleLongDate);
                 // remove "de" in spanish and "den" and ":e" in swedish
                 monthName.replace(QRegExp("\\sde[n]?\\s")," ");
                 monthName.remove(QRegExp(":e"));
                 QStringList sl = monthName.remove(QRegExp("(\\,|\\.|\\d+)")).split(' ', QString::SkipEmptyParts);
                 monthName = sl.at(1);
                 if ( monthName == MonthPicked )
                    monthPos = i ;
            }

            QDateTime pickedDate;
            pickedDate.setDate(QDate(YearPicked,monthPos,DayPicked) );
	    pickedDate.setTime(tiempo);
	    if (pickedDate < QDateTime::currentDateTime())
            {
                // in the past
    		ui->checkBox->setEnabled(false);
    		ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    		ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(false);
            }
	    else
            {
    		ui->checkBox->setEnabled(true);
    		ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
    		ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(true);
            }
        }

    }
    delete hw;

}

void NewAlarm::on_date_pushButton_pressed()
{
    QString dateDisplayed = ui->date_pushButton->valueText();
    QStringList sl = dateDisplayed.remove(QRegExp("(\\,|\\.|^\\w+|\\D+$)")).split(' ', QString::SkipEmptyParts);
    QString DayPicked;
    QString MonthPicked;
    QString YearPicked;
    for (int i=0; i<sl.size(); ++i)
    {
        // search through button text (minus weekdayname) for match
	QRegExp rx_year("\\d{4}");
	if ( rx_year.exactMatch(sl.at(i)) )
		YearPicked = sl.at(i);
	QRegExp rx_month("\\D+");
	if ( rx_month.exactMatch(sl.at(i)) )
		MonthPicked = sl.at(i);
	QRegExp rx_day("\\d{1,2}");
	if ( rx_day.exactMatch(sl.at(i)) )
		DayPicked = sl.at(i);
    }

    
    DialogDate* hw = new DialogDate(this, DayPicked, MonthPicked, YearPicked); 
    int result = hw->exec();

    if (result == QDialog::Accepted)
    {
        dateChoosen = true;
        ui->checkBox->setEnabled(true);
        int setDay = hw->dayres;
        int setMonth = hw->monthres;
        int setYear = hw->yearres;
        // enable alarm after accepting date
        enabled = true;
        ui->checkBox->setChecked(enabled);
 
        QDateTime date(QDate(setYear, setMonth, setDay));
        ui->date_pushButton->setValueText(formatDateTime(date.toTime_t(), FullDateShort));
        // check in past
        QString temp = ui->time_pushButton->valueText();
        int j = temp.indexOf(":");
        temp.remove(j, temp.length()-j);
        int hourval = temp.toInt();
        temp = ui->time_pushButton->valueText();
        j = temp.indexOf(":");
        temp.remove(0, j+1);
        int minuteval = temp.left(2).toInt();
        QString localPMtxt = QLocale::system().pmText();

        if (ui->time_pushButton->valueText().contains(localPMtxt) )
            hourval = hourval + 12;
        QDateTime pickedDateTime;
        pickedDateTime.setTime(QTime( hourval, minuteval ));
        pickedDateTime.setDate(QDate(setYear,setMonth,setDay) );
        if (pickedDateTime < QDateTime::currentDateTime())
        {
            // in the past
            ui->checkBox->setEnabled(false);
            ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
            ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(false);
        }
        else
        {
            ui->checkBox->setEnabled(true);
            ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
            ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(true);
        }
    }
    delete hw;
}

QString NewAlarm::longdate(QString data)
{

    QString localPMtxt = QLocale::system().pmText();
    // ade localPMtxt.remove(QRegExp("(\\,|\\.)"));
    QString localAMtxt = QLocale::system().amText();
    // ade localAMtxt.remove(QRegExp("(\\,|\\.)"));
    if ( (data.contains(localAMtxt)) || (data.contains(localPMtxt)) )
        return localAMtxt;
    else
        return "no";

}


void NewAlarm::on_repeat_pushButton_pressed()
{
    Dialog* hw = new Dialog(this, ui->repeat_pushButton->statusTip(), wday);
    int result = hw->exec();

    if (result == QDialog::Accepted)
    {
        QString lista;
        wday = 0;

        lista = hw->result.join(",");

        for (int i = 0; i < hw->result.count(); i ++)
            wday |= (1 << (hw->result.at(i).toInt()) % 7);

        if ( lista.contains("8") )
        {
            wday = ALARM_RECUR_WDAY_ALL;
            lista.clear();
            lista.append("8");
        }

        if ( !lista.contains("0") )
        {
            // a repeating alarm; "active checkbox" and "done" can always be enabled
            ui->checkBox->setEnabled(true);
            ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
            ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(true);
            if ( !ui->date_pushButton->isHidden() )
            {
                ui->date_pushButton->hide();
                this->setMinimumHeight(this->height() - 75);
                this->setMaximumHeight(this->height() - 75);
            }
        }
        else
        {
            if ( ui->date_pushButton->isHidden() )
            {
                this->setMinimumHeight(this->height() + 75);
                this->setMaximumHeight(this->height() + 75);
                ui->date_pushButton->show();
            }
        }

        ui->repeat_pushButton->setValueText(lista);
        // we have a change, so make active by default
        ui->checkBox->setChecked(true);
    }

    delete hw;
}

void NewAlarm::on_alsound_pushButton_pressed()
{
    AlarmSndPick *hw = new AlarmSndPick(this);
    hw->exec();
    if ( hw->selected != "nothing" )
    {
           sndFile = hw->selected;
           QString tmp = hw->selected;  // the full path/filename

           if ( tmp == "/usr/share/sounds/ui-clock_alarm_default.aac" )
   	        ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone1"));
           else if ( tmp == "/usr/share/sounds/ui-clock_alarm_2.aac" )
		ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone2"));
           else if ( tmp == "/usr/share/sounds/ui-clock_alarm_3.aac" )
                ui->alsound_pushButton->setValueText(_("cloc_fi_set_alarm_tone3"));
           else
	        ui->alsound_pushButton->setValueText( QFileInfo(sndFile.remove(QRegExp("\\.ogg$|\\.mp3$|\\.aac$" ))).fileName() );
	   sndFile = tmp;
    }
    delete hw;
}

void NewAlarm::on_buttonBox_clicked(QAbstractButton* button)
{
    intl("osso-connectivity-ui");
    if ( button->text() == _("conn_bd_devices_delete") )
    {
	// really remove alarm
        deleted = 1;
        removeAlarm(realcookie);
    }
    else
    {
        deleted = 0;

        if ( realcookie != 0  )
            alarmd_event_del(realcookie); // remove cookie if exists

        addAlarm();
    }

    intl("osso-clock");
    this->accept();

}

void NewAlarm::on_buttonBox_2_clicked(QAbstractButton* button)
{
    on_buttonBox_clicked(button);
}

void NewAlarm::removeAlarm(long cookie)
{
    if( alarmd_event_del(cookie) != -1 )
    {
        //qDebug() << "Deleted!";
        /*QSettings set1("cepiperez", "worldclock");
        set1.remove(QString::number(cookie));
        set1.sync();
        QSettings set2( "/var/cache/alarmd/alarm_queue.ini", QSettings::IniFormat );
        QString hex;
        hex.setNum(realcookie,16);
        while (hex.length()<8)
            hex = "0" + hex;
        hex = "#" + hex;
        qDebug() << hex;
        set2.remove(QString::number(realcookie));
        set2.sync();
        QSettings set3( "/var/cache/alarmd/alarm_queue.ini.bak", QSettings::IniFormat );
        set3.remove(QString::number(realcookie));
        set3.sync();*/

    }
}

void NewAlarm::addAlarm()
{

    intl("osso-clock");

    name = ui->lineEdit->text();

    time = ui->time_pushButton->valueText();
    enabled = ui->checkBox->isChecked();
    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();

    alarm_event_t * event = 0;
    alarm_action_t * act = 0;
    // Setup
    event = alarm_event_create(); // a new alarm
    // The application id field is needed for every alarm you set
    alarm_event_set_alarm_appid(event, "worldclock_alarmd_id");
    // Alarm description displayed when the alarm is triggered
    QByteArray ba = name.toUtf8();
    const char *str1 = ba.data();
    alarm_event_set_message(event, str1);
    /* Set the textdomain to be used while making
     * translation lookups */
    alarm_event_set_attr_string(event, "textdomain", TEXTDOMAIN);
    /* Set the title of the alarm. */
    //alarm_event_set_title(event, "Alarm"); "cloc_ti_alarm_notification_title"

    alarm_event_set_alarm_appid(event,"worldclock_alarmd_id");

    // Snooze action
    act = alarm_event_add_actions(event, 1);
    alarm_action_set_label(act, SNOOZE_BUTTON_KEY);
    act->flags = ALARM_ACTION_WHEN_RESPONDED;
    act->flags |= ALARM_ACTION_TYPE_SNOOZE;

    if ( !enabled )
        event->flags |= ALARM_EVENT_DISABLED;

    event->flags |= ALARM_EVENT_ACTDEAD;
    event->flags |= ALARM_EVENT_SHOW_ICON;
    event->flags |= ALARM_EVENT_POSTPONE_DELAYED;

    intl("osso-clock");

    if (wday)
        event->flags |= ALARM_EVENT_BACK_RESCHEDULE;

    // Acknowledge action
    act = alarm_event_add_actions(event, 1);
    alarm_action_set_label(act, STOP_BUTTON_KEY);

    if(!wday)
    {
       act->flags |= ALARM_ACTION_WHEN_RESPONDED;
       act->flags |= ALARM_ACTION_TYPE_DISABLE;
    } else {
       act->flags = ALARM_ACTION_WHEN_RESPONDED;
       act->flags |= ALARM_ACTION_TYPE_NOP;
    }

    //get time from button
    QString temp = ui->time_pushButton->valueText();
    int j = temp.indexOf(":");
    temp.remove(j, temp.length()-j);
    int hourval = temp.toInt();
    temp = ui->time_pushButton->valueText();
    j = temp.indexOf(":");
    temp.remove(0, j+1);
    int minuteval = temp.left(2).toInt();
    QString tmpx = ui->time_pushButton->valueText();
    if ( tmpx.contains(localPMtxt) && hourval!=12 )
        hourval = hourval+12;
    if ( tmpx.contains(localAMtxt) && hourval==12 )
        hourval = 0;

    QDateTime alarmDate;
    alarmDate.setTime(QTime( hourval, minuteval ));

    if ( ! ui->date_pushButton->isHidden() && (dateChoosen || !wday) )
    {
        QString dateDisplayed = ui->date_pushButton->valueText();
        QStringList sl = dateDisplayed.remove(QRegExp("(\\,|\\.|^\\w+|\\D+$)")).split(' ', QString::SkipEmptyParts);
        int DayPicked = 1;
        QString MonthPicked;
        int YearPicked = 1900;

        for (int i=0; i<sl.size(); ++i)
        {
            // search through button text (minus weekdayname) for match
            QRegExp rx_year("\\d{4}");
            if ( rx_year.exactMatch(sl.at(i)) )
                YearPicked = sl.at(i).toInt();
            QRegExp rx_month("\\D+");
            if ( rx_month.exactMatch(sl.at(i)) )
                MonthPicked = sl.at(i);
            QRegExp rx_day("\\d{1,2}");
            if ( rx_day.exactMatch(sl.at(i)) )
                DayPicked = sl.at(i).toInt();
        }

        int monthPos=1;
        for (int i=1; i<13; ++i)
        {
            QString num = QString::number(i);
	    QString monthName = QDateTime::fromString(num,"M").toString(Qt::DefaultLocaleLongDate);
            // remove "de" in spanish and "den" and ":e" in swedish
            monthName.replace(QRegExp("\\sde[n]?\\s")," ");
            monthName.remove(QRegExp(":e"));
            QStringList sl = monthName.remove(QRegExp("(\\,|\\.|\\d+)")).split(' ', QString::SkipEmptyParts);
            monthName = sl.at(1);
            if ( monthName == MonthPicked )
               monthPos = i ;
        }

        alarmDate.setDate(QDate(YearPicked,monthPos,DayPicked) );
        event->alarm_time = alarmDate.toTime_t();
        event->alarm_tm.tm_hour = -1;
        event->alarm_tm.tm_min = -1;
    }

    event->alarm_time = alarmDate.toTime_t(); //needed? looks like it...

    if( !wday ) {
        if (! dateChoosen) {
            event->alarm_time = -1;
            event->alarm_tm.tm_hour = alarmDate.time().hour();
            event->alarm_tm.tm_min = alarmDate.time().minute();
        }
    }
    else
    {
        //qDebug("Using the new recurrence API");
        event->recur_count = -1;
        event->recur_secs = 0; // We re not using this way for recurrence
        alarm_recur_t* recur = alarm_event_add_recurrences(event, 1);
        // Set event time
        recur->special = ALARM_RECUR_SPECIAL_NONE;
        recur->mask_mon = ALARM_RECUR_MON_ALL;
        recur->mask_mday = ALARM_RECUR_MDAY_ALL;
        recur->mask_hour = (1ul << alarmDate.time().hour());
        recur->mask_min = (1ull << alarmDate.time().minute());

        //qDebug() << "DIAS: " << ldays;
        recur->mask_wday = wday;
        Q_ASSERT(event->recurrence_cnt == 1);
    }

    // set the alarm signal
    if (sndFile.isEmpty())
    {
	// noting changed and not a default tone: so get the custom one
    	GConfItem *item = new GConfItem("/apps/clock/alarm-custom");
    	sndFile = item->value().toString();
    }

    QByteArray qba = sndFile.toUtf8();
    const char* mySndFile = qba.data();
    alarm_event_set_sound(event, mySndFile);
    realcookie = alarmd_event_add(event); // add alarm
    // cleanup
    alarm_event_delete(event);

    event = alarmd_event_get(realcookie);
    if ( enabled ) {
        /* OMG !!! FIXME */
        showAlarmTimeBanner(event->trigger);
    }
    alarm_event_delete(event);

    dateChoosen = false;
    act = 0;
    event = 0;
}
