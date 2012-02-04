#include "qmaemo5rotator.h"
#include "newalarm.h"
#include "ui_newalarm.h"
#include "osso-intl.h"
#include "alarmd/libalarm.h"
#include "dialog.h"
#include "dialog2.h"
#include <QDateTime>
#include <QMaemo5TimePickSelector>
#include "valuebutton.h"
/* osso-clock text needed for snooze/stop button translation */
#define TEXTDOMAIN "osso-clock"
#define STOP_BUTTON_KEY "cloc_bd_stop"
#define SNOOZE_BUTTON_KEY "cloc_bd_alarm_notification_snooze"

static uint32_t daysToMask(QList<int> days)
{
    uint32_t mask = 0;
    foreach(const int& d, days) {
      switch(d) {
      case 1:
        mask |= ALARM_RECUR_WDAY_MON;
        break;
      case 2:
        mask |= ALARM_RECUR_WDAY_TUE;
        break;
      case 3:
        mask |= ALARM_RECUR_WDAY_WED;
        break;
      case 4:
        mask |= ALARM_RECUR_WDAY_THU;
        break;
      case 5:
        mask |= ALARM_RECUR_WDAY_FRI;
        break;
      case 6:
        mask |= ALARM_RECUR_WDAY_SAT;
        break;
      case 7:
        mask |= ALARM_RECUR_WDAY_SUN;
        break;
      case 8:
        mask |= ALARM_RECUR_WDAY_ALL;
        break;
      default:
        Q_ASSERT(0); // Should never go here
        mask |= ALARM_RECUR_WDAY_ALL;
        break;
      }
    }
    return mask;
}

NewAlarm::NewAlarm(QWidget *parent, bool edit, QString Aname,
                   QString Atime, QString Adays, bool Acheck, long Acook, bool show):

    QDialog(parent),
    ui(new Ui::NewAlarm)
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    ui->setupUi(this);

    isEditing = edit;
    deleted = 1;

    realcookie = Acook;
    time = Atime;
    name = Aname;
    days = Adays;
    enabled = Acheck;
    showed = show;

    if ( edit )
        this->setWindowTitle(_("cloc_ti_edit_alarm_title"));
    else
        this->setWindowTitle(_("clock_ti_new_alarm"));

    ui->lineEdit->setPlaceholderText(_("cloc_va_placeholder_title"));
    ui->pushButton->setStatusTip(_("cloc_fi_time"));
    ui->pushButton_2->setStatusTip(_("cloc_fi_repeat"));
    ui->pushButton_2->setValueText("0");
    ui->pushButton_2->setWhatsThis("date");
    ui->checkBox->setText(_("cloc_fi_active"));

    intl("osso-connectivity-ui");
    ui->buttonBox->button(QDialogButtonBox::Retry)->setText(_("conn_bd_devices_delete"));
    ui->buttonBox->button(QDialogButtonBox::Apply)->setText(_("conn_bd_receive_ok"));
    ui->buttonBox_2->button(QDialogButtonBox::Retry)->setText(_("conn_bd_devices_delete"));
    ui->buttonBox_2->button(QDialogButtonBox::Apply)->setText(_("conn_bd_receive_ok"));
    intl("osso-clock");
    ui->buttonBox->button(QDialogButtonBox::Retry)->setVisible(edit);
    ui->buttonBox_2->button(QDialogButtonBox::Retry)->setVisible(edit);

    /*QMaemo5TimePickSelector * picker = new QMaemo5TimePickSelector(ui->pushButton);
    ui->pushButton->setPickSelector(picker);*/

    if ( name != "" )
        ui->lineEdit->setText(name);

    if ( time != "" )
        ui->pushButton->setValueText(time);
    else
        ui->pushButton->setValueText( QLocale::system().toString( QTime::currentTime(), QLocale::ShortFormat) );

    if ( days != "" )
        ui->pushButton_2->setValueText(days);
    else
        ui->pushButton_2->setValueText(_("cloc_va_never"));

    if ( !isEditing )
        ui->checkBox->hide();

    ui->checkBox->setChecked(enabled);

    on_lineEdit_textChanged(name);

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

    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        ui->buttonBox->hide();
        ui->buttonBox_2->show();
        this->setMinimumHeight(310+space+space);
        this->setMaximumHeight(310+space+space);
    } else {
        ui->buttonBox_2->hide();
        ui->buttonBox->show();
        this->setMinimumHeight(230+space);
        this->setMaximumHeight(230+space);
    }

    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

}

void NewAlarm::on_pushButton_pressed()
{
    QString temp = ui->pushButton->valueText();
    int j = temp.indexOf(":");
    temp.remove(j, temp.length()-j);
    int val1 = temp.toInt();
    temp = ui->pushButton->valueText();
    j = temp.indexOf(":");
    temp.remove(0, j+1);
    int val2 = temp.left(2).toInt();
    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();

    bool ampm = false;
    if ( longdate(ui->pushButton->valueText()) != "no"  )
        ampm = true;

    bool am = true;
    if (ui->pushButton->valueText().contains(localPMtxt) )
        am = false;

    QString dam = localAMtxt;
    QString dpm = localPMtxt;

    Dialog2* hw = new Dialog2(this, val1, val2, ampm, am, dam, dpm);
    int result = hw->exec();

    if (result == QDialog::Accepted)
    {
        QTime tiempo;
        int hora = hw->res1;
        int mins = hw->res2;
        if ( (longdate(ui->pushButton->valueText())!="no") && (!hw->isam) )
        {
            hora = hw->res1+12;
        }
        tiempo.setHMS(hora, mins, 0);

        ui->pushButton->setValueText( QLocale::system().toString(tiempo, QLocale::ShortFormat));
    }
    delete hw;

}

QString NewAlarm::longdate(QString data)
{

    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();
    if ( (data.contains(localAMtxt)) || (data.contains(localPMtxt)) )
        return localAMtxt;
    else
        return "no";

}


void NewAlarm::on_pushButton_2_pressed()
{
    Dialog* hw = new Dialog(this, ui->pushButton_2->statusTip(), ui->pushButton_2->valueText());
    int result = hw->exec();

    if (result == QDialog::Accepted)
    {
        QString lista;
        for (int i=0; i < hw->result.count(); ++i)
        {
            if ( lista.count() > 0 )
                lista.append(",");
            lista.append( hw->result.at(i) );
        }

        if ( lista.contains("8") )
        {
            lista.clear();
            lista.append("8");
        }

        ui->pushButton_2->setValueText(lista);
    }
    delete hw;

}


void NewAlarm::on_buttonBox_clicked(QAbstractButton* button)
{
    intl("osso-connectivity-ui");
    if ( button->text() == _("conn_bd_devices_delete") )
    {
        deleted = 1;
        removeAlarm(realcookie);
    }
    else
    {
        deleted = 0;

        if ( realcookie != 0 )
            alarmd_event_del(realcookie);

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
    days = ui->pushButton_2->valueText();
    time = ui->pushButton->valueText();
    enabled = ui->checkBox->isChecked();
    QString localPMtxt = QLocale::system().pmText();
    QString localAMtxt = QLocale::system().amText();

    alarm_event_t * event = 0;
    alarm_action_t * act = 0;


    //qDebug() << "!Adding alarm: enabled: " << enabled;

    // Setup
    event = alarm_event_create();
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
    alarm_event_set_title(event, "Alarm");

    alarm_event_set_alarm_appid(event,"worldclock_alarmd_id");


    // Snooze action
    act = alarm_event_add_actions(event, 1);
    //alarm_action_set_label(act, _("cloc_bd_alarm_notification_snooze").toUtf8());
    alarm_action_set_label(act, SNOOZE_BUTTON_KEY);
    act->flags = ALARM_ACTION_WHEN_RESPONDED;
    act->flags |= ALARM_ACTION_TYPE_SNOOZE;

    if ( !enabled )
        event->flags |= ALARM_EVENT_DISABLED;

    event->flags |= ALARM_EVENT_ACTDEAD;
    event->flags |= ALARM_EVENT_SHOW_ICON;
    event->flags |= ALARM_EVENT_POSTPONE_DELAYED;

    QList<int> ldays;
    QString tmp = days;

    intl("osso-clock");
    if ( tmp == "0" )
    {
        ldays << 0;

    }
    else if ( tmp == "8" )
    {
        event->flags |= ALARM_EVENT_BACK_RESCHEDULE;
        ldays << 8;
    }
    else
    {
        event->flags |= ALARM_EVENT_BACK_RESCHEDULE;
        if ( tmp.contains("1") )
            ldays << 1;
        if ( tmp.contains("2") )
            ldays << 2;
        if ( tmp.contains("3") )
            ldays << 3;
        if ( tmp.contains("4") )
            ldays << 4;
        if ( tmp.contains("5") )
            ldays << 5;
        if ( tmp.contains("6") )
            ldays << 6;
        if ( tmp.contains("7") )
            ldays << 7;
    }
    // Acknowledge action
    act = alarm_event_add_actions(event, 1);
    //alarm_action_set_label(act, _("cloc_bd_stop").toUtf8());
    alarm_action_set_label(act, STOP_BUTTON_KEY);
    if(ldays.first() == 0) {
       act->flags |= ALARM_ACTION_WHEN_RESPONDED;
       act->flags |= ALARM_ACTION_TYPE_DISABLE;
    } else {
       act->flags = ALARM_ACTION_WHEN_RESPONDED;
       act->flags |= ALARM_ACTION_TYPE_NOP;
    }

    QString temp = ui->pushButton->valueText();
    int j = temp.indexOf(":");
    temp.remove(j, temp.length()-j);
    int val1 = temp.toInt();
    temp = ui->pushButton->valueText();
    j = temp.indexOf(":");
    temp.remove(0, j+1);
    int val2 = temp.left(2).toInt();
    QString tmpx = ui->pushButton->valueText();
//    tmpx.remove(".");
    if ( tmpx.contains(localPMtxt) && val1!=12 )
        val1 = val1+12;
    if ( tmpx.contains(localAMtxt) && val1==12 )
        val1 = 0;

    QDateTime currDate;
    currDate.setTime(QTime( val1, val2 ));
    event->alarm_time = currDate.toTime_t();

    /*if(days.first() == 8) {
      event->recur_secs = 86400; // 24 hours
      event->recur_count = -1; // Reoccur infinitely
      event->alarm_time = toTime_t(currDate.time()); // Set event time
      alarm_recur_t* recur = alarm_event_add_recurrences(event, 1);
      recur->mask_wday = ALARM_RECUR_WDAY_ALL;
    } else {*/
      if(ldays.first() == 0) {
        event->alarm_time = -1;
        event->alarm_tm.tm_hour = currDate.time().hour();
        event->alarm_tm.tm_min = currDate.time().minute();
      } else {
        //qDebug("Using the new recurrence API");
        event->recur_count = -1;
        event->recur_secs = 0; // We re not using this way for recurrence
        alarm_recur_t* recur = alarm_event_add_recurrences(event, 1);
        // Set event time
        recur->special = ALARM_RECUR_SPECIAL_NONE;
        recur->mask_mon = ALARM_RECUR_MON_ALL;
        recur->mask_mday = ALARM_RECUR_MDAY_ALL;
        recur->mask_hour = (1ul << currDate.time().hour());
        recur->mask_min = (1ull << currDate.time().minute());

        //qDebug() << "DIAS: " << ldays;
        recur->mask_wday = daysToMask(ldays);
        Q_ASSERT(event->recurrence_cnt == 1);
      }
    //}
    /* Send notification on banner */
    QDateTime now = QDateTime::currentDateTime();

    QDateTime alarmTime = QDateTime( now.date(), QTime(currDate.time().hour(),currDate.time().minute(),0) );
    QString AlarmText = _("cloc_notify_alarm_set");
    if(((ldays.first() == 0) || (ldays.first() == 8)) || (( alarmTime > now ) && \
    (ldays.contains(QDate::currentDate().dayOfWeek()))) ||
    ((ldays.contains(QDate::currentDate().addDays(1).dayOfWeek())) && ( alarmTime < now )))
    {
         int secsDiff = now.secsTo(alarmTime);
         QDate nullDate(2012, 1, 1);
         QDateTime nullTime = QDateTime::QDateTime(nullDate);
         QDateTime upTime = nullTime.addSecs(secsDiff);
         QVariant days = nullTime.daysTo(upTime);
         QTime timeTime = upTime.time();
         int hours = timeTime.hour();
         int minutes = timeTime.minute();
         int seconds = timeTime.second();
         if ( seconds > 30 ) 
            minutes = minutes + 1; 
         QString hhmm = QString("%1:%2").arg(QString().sprintf("%02d",hours)).arg(QString().sprintf("%02d",minutes));
         AlarmText.replace("%s",hhmm);
    } else {
         AlarmText = _("cloc_notify_alarm_set_days");
         AlarmText.replace("%s" ,"");
         int curDayOfWeek = QDate::currentDate().dayOfWeek();
         int i;
         int nextDayOfWeek = curDayOfWeek;
         for(i= 1; i < 8; i++)
         {
            if ( nextDayOfWeek == 7 )
                nextDayOfWeek = 1;
            else
		nextDayOfWeek++;
            if ( tmp.contains(QString().sprintf("%1d",nextDayOfWeek) )) {
                if (i == 1 ) {
 	           AlarmText = AlarmText + _("cloc_va_amount_day");
                   AlarmText.replace("%d",QString().sprintf("%1d",i));
                   break;
                } else {
                   AlarmText = AlarmText + ngettext("cloc_va_amount_day", "cloc_va_amount_days", i);
                   AlarmText.replace("%d",QString().sprintf("%1d",i));
                   break;
                }
            }
         }
    }
    if ( enabled ) {
       QString notificationDuration = "dbus-send --type=method_call --dest=org.freedesktop.Notifications \
                        /org/freedesktop/Notifications org.freedesktop.Notifications.SystemNoteInfoprint \
                        string:\"" + AlarmText + "\"";
       QProcess::startDetached(notificationDuration);
    }
 


    alarm_event_set_sound(event, NULL);

    realcookie = alarmd_event_add(event);

    // Cleanup
    alarm_event_delete(event);
    act = 0;
    event = 0;

    //cookie = event->cookie;
    //qDebug() << "Guardando alarma nueva" << realcookie;

}


//void NewAlarm::on_lineEdit_textChanged(QString text)
void NewAlarm::on_lineEdit_textChanged(QString)
{
    /*if ( text == "" )
    {
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(false);
    }
    else
    {
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
        ui->buttonBox_2->button(QDialogButtonBox::Apply)->setEnabled(true);
    }*/
}
