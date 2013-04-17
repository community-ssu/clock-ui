#include "citydetail.h"
#include "ui_citydetail.h"
#include "osso-intl.h"
#include "clockd/libtime.h"
#include <QMaemo5Style>
#include <QDateTime>
#include <QSettings>
#include <QTimer>

static const char *getHildonTranslation(const char *string)
{
     setlocale (LC_ALL, "");
     const char *translation = ::dgettext("hildon-libs", string);
     if (qstrcmp(string, translation) == 0)
         return 0;
     return translation;
}

const char *hldon24hFormat = getHildonTranslation("wdgt_va_24h_time");
const char *hldon12hAMFormat = getHildonTranslation("wdgt_va_12h_time_am");
const char *hldon12hPMFormat = getHildonTranslation("wdgt_va_12h_time_pm");
const char *hldonHHFormat = getHildonTranslation("wdgt_va_24h_hours");

bool HH24true;
int OffSetSecs;
QString CityCode;

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

CityDetail::CityDetail(QWidget *parent, QString city_country, QString city_code, QString external_gmt, QString local_gmt, bool hh24, int offset_sec ) :
    QMainWindow(parent),
    ui(new Ui::CityDetail)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setWindowTitle(_("cloc_ti_world_clock_details"));

    HH24true = hh24;
    OffSetSecs = offset_sec;
    CityCode = city_code;
    // set secondary colors
    QColor secondaryColor = QMaemo5Style::standardColor("SecondaryTextColor");
    ui->time_diff->setStyleSheet(QString("color: rgb(%1, %2, %3);")
	                  .arg(secondaryColor.red())
			  .arg(secondaryColor.green())
                          .arg(secondaryColor.blue()));
    ui->city_country->setStyleSheet(QString("color: rgb(%1, %2, %3);")
	                  .arg(secondaryColor.red())
			  .arg(secondaryColor.green())
                          .arg(secondaryColor.blue()));
    ui->external_time->setStyleSheet(QString("color: rgb(%1, %2, %3);")
	                  .arg(secondaryColor.red())
			  .arg(secondaryColor.green())
                          .arg(secondaryColor.blue()));
    ui->loc_time->setStyleSheet(QString("color: rgb(%1, %2, %3);")
	                  .arg(secondaryColor.red())
			  .arg(secondaryColor.green())
                          .arg(secondaryColor.blue()));
    // set texts
    ui->city_country->setText("(" + city_country + ")");
    ui->gmt_offset->setText(external_gmt);
    ui->loc_time_gmt->setText(_("cloc_fi_local_time") + " " + local_gmt); 
    ui->ext_time_header->setText(_("cloc_fi_remote_time"));
    ui->time_diff_header->setText(_("cloc_fi_time_difference"));
    // external GMT offset
    int offset_minutes_ext_gmt = 0;
    int offset_hours_ext_gmt = 0;
    QString offset_time_ext_gmt = ui->gmt_offset->text().section( " ", 1, 1 );
    if ( offset_time_ext_gmt.contains(":"))
    {
    	// we also have minutes
    	offset_hours_ext_gmt = offset_time_ext_gmt.section( ":", 0, 0 ).toInt();
	offset_hours_ext_gmt = offset_hours_ext_gmt * 60;
    	offset_minutes_ext_gmt = offset_time_ext_gmt.section( ":", 1, 1 ).toInt();
    }
    else
    {
    	offset_hours_ext_gmt = offset_time_ext_gmt.toInt();
	offset_hours_ext_gmt = offset_hours_ext_gmt * 60;
    }
    // local GMT offset
    int offset_minutes_loc_gmt = 0;
    int offset_hours_loc_gmt = 0;
    QString offset_time_loc_gmt = local_gmt.section( " ", 1, 1 );
    QString timediff_txt = "";
    if ( offset_time_loc_gmt.contains(":"))
    {
    	// we also have minutes
    	offset_hours_loc_gmt = offset_time_loc_gmt.section( ":", 0, 0 ).toInt();
    	offset_hours_loc_gmt = offset_hours_loc_gmt * 60;
    	offset_minutes_loc_gmt = offset_time_loc_gmt.section( ":", 1, 1 ).toInt();
    }
    else
    {
    	offset_hours_loc_gmt = offset_time_loc_gmt.toInt();
    	offset_hours_loc_gmt = offset_hours_loc_gmt * 60;
    }

    QString tot_diff_txt = "";
    int tot_diff_min = (- offset_minutes_loc_gmt + offset_minutes_ext_gmt) %60;
    int tot_diff_hour = (-offset_hours_loc_gmt + offset_hours_ext_gmt )/60;
    timediff_txt = QString::fromUtf8(ngettext("cloc_va_amount_hour", "cloc_va_amount_hours", abs(tot_diff_hour)));
    if (tot_diff_hour > 0)
	tot_diff_txt = "+" + QString::number(tot_diff_hour);
    else 
	tot_diff_txt = QString::number(tot_diff_hour);
    if (tot_diff_min != 0)
    {    
	timediff_txt = timediff_txt + " " + _("cloc_va_diff_hours_mins");
	timediff_txt.replace("%s %d",QString::number(abs(tot_diff_min)));
    }
    timediff_txt.replace("%+d",tot_diff_txt);
    
    ui->time_diff->setText(timediff_txt);
    intl("osso-connectivity-ui");
    ui->actionDelete->setText(_("conn_bd_devices_delete"));
    intl("osso-clock");
    updateInfo();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateInfo()));
    timer->start(4000);
}

CityDetail::~CityDetail()
{
    delete ui;
}

void CityDetail::on_actionDelete_triggered()
{
    // remove city from the config file
    QSettings settings("worldclock", "worldclock");
    QStringList citylist = settings.value("Cities",QStringList()).toStringList();
    citylist.removeOne(CityCode);
    settings.setValue("Cities", citylist);
    settings.sync();
    this->close();
}

void CityDetail::updateInfo()
{
       // refesh screen info
       QDate curdate = QDate::currentDate();
       QDateTime ext_datetime = QDateTime::currentDateTime();
       ext_datetime = ext_datetime.toUTC();
       ext_datetime = ext_datetime.addSecs( -OffSetSecs );
       QString CurrTime;
       QString ExtTime;
       if (HH24true)
       {
          CurrTime = formatHildonDate(QDateTime::currentDateTime(), hldon24hFormat);
          ExtTime = formatHildonDate(ext_datetime, hldon24hFormat);
       }
       else
       {
          QString HH = formatHildonDate(QDateTime::currentDateTime(), hldonHHFormat);
          if ( HH.toInt() > 11 )
          {
               CurrTime = formatHildonDate(QDateTime::currentDateTime(), hldon12hPMFormat);
	  }
          else
          {
               CurrTime = formatHildonDate(QDateTime::currentDateTime(), hldon12hAMFormat);
	  }
          HH = formatHildonDate(ext_datetime, hldonHHFormat);
          if ( HH.toInt() > 11 )
          {
               ExtTime = formatHildonDate(ext_datetime, hldon12hPMFormat);
	  }
          else
          {
               ExtTime = formatHildonDate(ext_datetime, hldon12hAMFormat);
	  }
       }

       CurrTime = CurrTime + " - " + curdate.toString(Qt::DefaultLocaleLongDate);
       ExtTime = ExtTime + " - " + ext_datetime.date().toString(Qt::DefaultLocaleLongDate);
       ui->loc_time->setText(CurrTime);
       ui->external_time->setText(ExtTime);
}
