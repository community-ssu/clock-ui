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
/* osso-clock text needed for snooze/stop button translation */
#define TEXTDOMAIN "osso-clock"
#define STOP_BUTTON_KEY "cloc_bd_stop"
#define SNOOZE_BUTTON_KEY "cloc_bd_alarm_notification_snooze"

static const char *getHildonTranslation(const char *string)
{
     setlocale (LC_ALL, "");
     setlocale(LC_TIME, "C");
     const char *translation = ::dgettext("hildon-libs", string);
     if (qstrcmp(string, translation) == 0)
         return 0;
     return translation;
}

const char *hildonDateDayName_Short = getHildonTranslation("wdgt_va_date_day_name_short");
// const char *hildonDateLong = getHildonTranslation("wdgt_va_date_long");
const char *hildonAMfmt = getHildonTranslation("wdgt_va_12h_time_am");
const char *hildonPMfmt = getHildonTranslation("wdgt_va_12h_time_pm");
const char *hildon24fmt = getHildonTranslation("wdgt_va_24h_time");
const char *hildonHHfmt = getHildonTranslation("wdgt_va_24h_hours");
QString dateSep = QDate::currentDate().toString(Qt::SystemLocaleShortDate).remove(QRegExp("\\d+")).at(0);
bool dateChoosen = false;
bool fromEdit = false;

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

    // get the current time format
    GConfItem *HH24 = new GConfItem("/apps/clock/time-format");
    bool HH24true = HH24->value().toBool();


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
    ui->time_pushButton->setStatusTip(_("cloc_fi_time"));
    ui->date_pushButton->setStatusTip(_("dati_fi_pr_date"));
    ui->repeat_pushButton->setStatusTip(_("cloc_fi_repeat"));
    ui->repeat_pushButton->setValueText("0");
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
    if ( name != "" )
        ui->lineEdit->setText(name);

    // set time
    if ( time != "" )
        ui->time_pushButton->setValueText(time);
    else
    {
        if (HH24true)
            ui->time_pushButton->setValueText(formatHildonDate(QDateTime::currentDateTime(), hildon24fmt) );
        else
        {
           QString HH = formatHildonDate(QDateTime::currentDateTime(), hildonHHfmt);
           if ( HH.toInt() > 11 )
                ui->time_pushButton->setValueText(formatHildonDate(QDateTime::currentDateTime(), hildonPMfmt) );
           else
                ui->time_pushButton->setValueText(formatHildonDate(QDateTime::currentDateTime(), hildonAMfmt) );
        }
    }

    // Make default date tomorrow
    QString currDayNameShort = formatHildonDate(QDateTime::currentDateTime().addDays(1), hildonDateDayName_Short);
    QString currDayLong = QDate::currentDate().addDays(1).toString(Qt::DefaultLocaleLongDate);

    // a specific date is already set
    if ( days.contains(dateSep))
    {
        ui->date_pushButton->setValueText(days);
	QString temp = ui->date_pushButton->valueText();
	QString subdays;
	QString submonths;
	QString subyears;
	QString testdate = QLocale::system().toString(QDateTime::currentDateTime(), QLocale::LongFormat);
	testdate.remove(QRegExp("(\\,|\\.|\' |^\\w+)"));
	testdate.remove(QRegExp("^\\s"));
	QRegExp rx_month("^\\D.+");
	if ( rx_month.exactMatch(testdate) )
	{
		// month first
                subdays = temp.section( dateSep, 1, 1 );
                submonths = temp.section( dateSep, 0, 0 );
	}
	else
	{
                subdays = temp.section( dateSep, 0, 0 );
                submonths = temp.section( dateSep, 1, 1 );
	}
	subyears = temp.section( dateSep, 2, 2 );
	currDayNameShort = formatHildonDate(QDateTime::fromString(subyears+submonths+subdays,"yyyyMd"), hildonDateDayName_Short);
	// we only want the shortdayname itself
        QStringList sl = currDayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
        currDayNameShort = sl.at(0);
	// The long date string
	currDayLong = QDate::fromString(subyears+submonths+subdays,"yyyyMd").toString(Qt::DefaultLocaleLongDate);
	//QString currDayLong = formatHildonDate(QDateTime::fromString(subyears+submonths+subdays,"yyyyMd"), hildonDateLong);
	// remove "de" in spanish and "den" and ":e" in swedish
	currDayLong.replace(QRegExp("\\sde[n]?\\s")," ");
	currDayLong.remove(QRegExp(":e"));
	// we will use the shortdayname
	currDayLong.replace(QRegExp("^\\w+\\b"),currDayNameShort);

        // ui->date_pushButton->setValueText( currDayLong );
    }
    else if ( isEditing && ! days.contains(dateSep))
    {
	QStringList orgTime = time.split(':', QString::SkipEmptyParts);
	int hourOrg = orgTime.at(0).toInt();
	int minuteOrg = orgTime.at(1).toInt();

	QTime timeNow = QTime::currentTime();
	int hourNow = timeNow.hour();
	int minuteNow = timeNow.minute();
	if ((hourOrg > hourNow) || (hourOrg == hourNow && minuteOrg > minuteNow))
	{
		currDayNameShort = formatHildonDate(QDateTime::currentDateTime(), hildonDateDayName_Short);
		currDayLong = QDate::currentDate().toString(Qt::DefaultLocaleLongDate);
	}
    }
     
    // we only want the shortdayname itself
    QStringList sl = currDayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
    currDayNameShort = sl.at(0);

    // remove "de" in spanish and "den" and ":e" in swedish
    currDayLong.replace(QRegExp("\\sde[n]?\\s")," ");
    currDayLong.remove(QRegExp(":e"));
    // we will use the shortdayname
    currDayLong.replace(QRegExp("^\\w+\\b"),currDayNameShort);

    ui->date_pushButton->setValueText( currDayLong );

    if ( (days != "") && (days != "0" && !days.contains(dateSep) ))
    {
	if ( !ui->date_pushButton->isHidden() )
        {
       	    this->setMinimumHeight(this->height() - 75);
            this->setMaximumHeight(this->height() - 75);
        }
        ui->date_pushButton->hide();
    }

    if ( days != "" )
        ui->repeat_pushButton->setValueText(days);
    else
        ui->repeat_pushButton->setValueText(_("cloc_va_never"));

    if ( !isEditing )
        ui->checkBox->hide();

    ui->checkBox->setChecked(enabled);

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

    int startspace_portrait = 385;
    int startspace_landscape = 305;
    if ( ui->date_pushButton->isHidden() )
    {
        startspace_portrait = 310;
        startspace_landscape = 230;
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
    int val1 = temp.toInt();
    temp = ui->time_pushButton->valueText();
    j = temp.indexOf(":");
    temp.remove(0, j+1);
    int val2 = temp.left(2).toInt();
    QString localPMtxt = QLocale::system().pmText();
    localPMtxt.remove(QRegExp("(\\,|\\.)"));
    QString localAMtxt = QLocale::system().amText();
    localAMtxt.remove(QRegExp("(\\,|\\.)"));

    bool ampm = false;
    if ( longdate(ui->time_pushButton->valueText()) != "no"  )
        ampm = true;

    bool am = true;
    if (ui->time_pushButton->valueText().contains(localPMtxt) )
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
	QString currDayLong = QDate::currentDate().toString(Qt::DefaultLocaleLongDate);
	// QString currDayLong = formatHildonDate(QDateTime::currentDateTime(), hildonDateLong);
	if ( ! dateChoosen && !days.contains(dateSep) )
	{
		// show if the alarm is still today or else tomorrow
		if ((hora > hourNow) || (hora == hourNow && mins > minuteNow))
		{
			// alarmtime > currenttime, one day back
			QString currDayNameShort = formatHildonDate(QDateTime::currentDateTime(), hildonDateDayName_Short);
			// we only want the shortdayname itself
			QStringList sl = currDayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
			currDayNameShort = sl.at(0);
			// remove "de" in spanish and "den" and ":e" in swedish
			currDayLong.replace(QRegExp("\\sde[n]?\\s")," ");
			currDayLong.remove(QRegExp(":e"));
			// we will use the shortdayname
			currDayLong.replace(QRegExp("^\\w+\\b"),currDayNameShort);
			// The long date string
		}
		else
		{
			// alarmtime < currenttime, one day plus
			QString currDayNameShort = formatHildonDate(QDateTime::currentDateTime().addDays(1), hildonDateDayName_Short);
			// we only want the shortdayname itself
			QStringList sl = currDayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
			currDayNameShort = sl.at(0);
			// The long date string
			currDayLong = QDate::currentDate().addDays(1).toString(Qt::DefaultLocaleLongDate);
			//currDayLong = formatHildonDate(QDateTime::currentDateTime().addDays(1), hildonDateLong);
			// remove "de" in spanish and "den" and ":e" in swedish
			currDayLong.replace(QRegExp("\\sde[n]?\\s")," ");
			currDayLong.remove(QRegExp(":e"));
			// we will use the shortdayname
			currDayLong.replace(QRegExp("^\\w+\\b"),currDayNameShort);
		}

		ui->date_pushButton->setValueText( currDayLong );
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
        int setDay = hw->dayres;
        int setMonth = hw->monthres;
        int setYear = hw->yearres;
        // enable alarm after accepting date
        enabled = true;
        ui->checkBox->setChecked(enabled);
 
	QString currDayNameShort = formatHildonDate(QDateTime::fromString(QString::number(setDay)+"."+QString::number(setMonth)+"."+QString::number(setYear),"d.M.yyyy"), hildonDateDayName_Short);
	// we only want the shortdayname itself
        QStringList sl = currDayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
        currDayNameShort = sl.at(0);
	// The long date string
	// QString currDayLong = formatHildonDate(QDateTime::fromString(QString::number(setDay)+"."+QString::number(setMonth)+"."+QString::number(setYear),"d.M.yyyy"), hildonDateLong);
	QString currDayLong = QDate::fromString(QString::number(setDay)+"."+QString::number(setMonth)+"."+QString::number(setYear),"d.M.yyyy").toString(Qt::DefaultLocaleLongDate);
	// remove "de" in spanish and "den" and ":e" in swedish
	currDayLong.replace(QRegExp("\\sde[n]?\\s")," ");
	currDayLong.remove(QRegExp(":e"));
	// we will use the shortdayname
	currDayLong.replace(QRegExp("^\\w+\\b"),currDayNameShort);

        ui->date_pushButton->setValueText( currDayLong );
    }
    delete hw;
}

QString NewAlarm::longdate(QString data)
{

    QString localPMtxt = QLocale::system().pmText();
    localPMtxt.remove(QRegExp("(\\,|\\.)"));
    QString localAMtxt = QLocale::system().amText();
    localAMtxt.remove(QRegExp("(\\,|\\.)"));
    if ( (data.contains(localAMtxt)) || (data.contains(localPMtxt)) )
        return localAMtxt;
    else
        return "no";

}


void NewAlarm::on_repeat_pushButton_pressed()
{
    Dialog* hw = new Dialog(this, ui->repeat_pushButton->statusTip(), ui->repeat_pushButton->valueText());
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
        if ( !lista.contains("0") )
	{
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
    days = ui->repeat_pushButton->valueText();
    time = ui->time_pushButton->valueText();
    enabled = ui->checkBox->isChecked();
    QString localPMtxt = QLocale::system().pmText();
    localPMtxt.remove(QRegExp("(\\,|\\.)"));
    QString localAMtxt = QLocale::system().amText();
    localAMtxt.remove(QRegExp("(\\,|\\.)"));

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
    //alarm_event_set_title(event, "Alarm");

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

    QList<int> ldays;
    QString tmp = days;

    intl("osso-clock");
    // specific date/time:
    if ( tmp.contains(dateSep) )
	tmp = "0";
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
    alarm_action_set_label(act, STOP_BUTTON_KEY);
    if(ldays.first() == 0) {
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
    int val1 = temp.toInt();
    temp = ui->time_pushButton->valueText();
    j = temp.indexOf(":");
    temp.remove(0, j+1);
    int val2 = temp.left(2).toInt();
    QString tmpx = ui->time_pushButton->valueText();
    if ( tmpx.contains(localPMtxt) && val1!=12 )
        val1 = val1+12;
    if ( tmpx.contains(localAMtxt) && val1==12 )
        val1 = 0;

    QDateTime currDate;
    currDate.setTime(QTime( val1, val2 ));

    if ( ! ui->date_pushButton->isHidden() && (dateChoosen || days.contains(dateSep)) )
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
            //QString monthName = formatHildonDate(QDateTime::fromString(num,"MM"), hildonDateLong);
            QStringList sl = monthName.remove(QRegExp("(\\,|\\.|\\d+)")).split(' ', QString::SkipEmptyParts);
            monthName = sl.at(1);
            if ( monthName == MonthPicked )
               monthPos = i ;
        }

        currDate.setDate(QDate(YearPicked,monthPos,DayPicked) );
        event->alarm_time = currDate.toTime_t();
        event->alarm_tm.tm_hour = -1;
        event->alarm_tm.tm_min = -1;
    }

    event->alarm_time = currDate.toTime_t(); //needed? looks like it...

      if( ldays.first() == 0 ) {
	if (! dateChoosen && !days.contains(dateSep)) {
            event->alarm_time = -1;
            event->alarm_tm.tm_hour = currDate.time().hour();
            event->alarm_tm.tm_min = currDate.time().minute();
	}
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
         QDateTime nullTime = QDateTime(nullDate);
         QDateTime upTime = nullTime.addSecs(secsDiff);
         QVariant days = nullTime.daysTo(upTime);
         QTime timeTime = upTime.time();
         int hours = timeTime.hour();
         int minutes = timeTime.minute();
         int seconds = timeTime.second();
         if ( seconds > 30 ) 
            minutes = minutes + 1; 
         QString hhmm = QString("%1:%2").arg(QString().sprintf("%02d",hours)).arg(QString().sprintf("%02d",minutes));
         // can happen is alarm is set within 30 secs
         hhmm.replace("23:60","24:00");
         hhmm.replace("00:60","01:00");
         AlarmText.replace("%s",hhmm);
    	 if ( event->alarm_time != -1 )
         {
		// specific alarms
         	int i = now.daysTo(currDate);
		if ( (currDate.toTime_t() - now.toTime_t()) > 86399 )
                {
			// difference more then one day (86400 seconds)
			AlarmText = _("cloc_notify_alarm_set_days");
			AlarmText.replace("%s" ,"");
			AlarmText = AlarmText + QString::fromUtf8(ngettext("cloc_va_amount_day", "cloc_va_amount_days", i));
			AlarmText.replace("%d",QString().sprintf("%1d",i));
		}
         }
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
                   AlarmText = AlarmText + QString::fromUtf8(ngettext("cloc_va_amount_day", "cloc_va_amount_days", i));
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

    dateChoosen = false;

    // Cleanup
    alarm_event_delete(event);
    act = 0;
    event = 0;

    //cookie = event->cookie;
    //qDebug() << "Guardando alarma nueva" << realcookie;

}
