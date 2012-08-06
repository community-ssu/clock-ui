#include <libnotify/notify.h>
#include "qmaemo5rotator.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alarmlist.h"
#include "osso-intl.h"
#include "newalarm.h"
#include "alarmbutton.h"
#include "maintdelegate.h"
#include "world.h"
#include "alsettings.h"
#include "gconfitem.h"
#include <QDateTime>
#include <libosso.h>
#include <QMaemo5Style>

#include <X11/Xlib.h>
#include <gtk-2.0/gdk/gdkx.h>
#include <gtk-2.0/gtk/gtk.h>
#include <gtk-2.0/gtk/gtkwidget.h>

static const char *getHildonTranslation(const char *string)
{
     setlocale (LC_ALL, "");
     const char *translation = ::dgettext("hildon-libs", string);
     if (qstrcmp(string, translation) == 0)
         return 0;
     return translation;
}

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

const char *hildon24hFormat = getHildonTranslation("wdgt_va_24h_time");
const char *hildon12hAMFormat = getHildonTranslation("wdgt_va_12h_time_am");
const char *hildon12hPMFormat = getHildonTranslation("wdgt_va_12h_time_pm");
const char *hildonHHFormat = getHildonTranslation("wdgt_va_24h_hours");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setWindowFlags(Qt::Window);
    ui->setupUi(this);

    this->setWindowTitle(_("cloc_ap_name"));

    // Read some variables
    // set background by default
    Bool BackgroundImg = true;
    QSettings settings("worldclock", "worldclock");
    if (settings.contains("Background"))
    	BackgroundImg = settings.value("Background").toBool();
    SecondsAdded = settings.value("SecondsAdded").toBool();
    // Compose image text
    intl("hildon-fm");
    QString ImageText = QString::fromUtf8(ngettext("sfil_va_number_of_objects_image", "sfil_va_number_of_objects_images", 1));
    ImageText.remove(QRegExp("%d\\W+"));
    ImageText[0] = ImageText[0].toUpper();
    // Compose seconds text
    intl("osso-display");
    QString SecondsText = QString::fromUtf8(ngettext("disp_va_gene_0", "disp_va_gene_1", 2));
    SecondsText.remove(QRegExp("%d\\W+"));
    SecondsText[0] = SecondsText[0].toUpper();
    intl("osso-clock");

    if ( BackgroundImg ) {
	    this->setAutoFillBackground(true);
	    QPalette pal2(palette());
	    pal2.setBrush(QPalette::Window, QPixmap("/etc/hildon/theme/backgrounds/clock.png"));
	    this->setPalette(pal2);
    }

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QMaemo5Style::standardColor("DefaultBackgroundColor"));
    ui->widget_3->setAutoFillBackground(true);
    ui->widget_3->setPalette(pal);

    pal.setBrush(QPalette::Active, QPalette::WindowText, QMaemo5Style::standardColor("DefaultTextColor"));
    ui->newAlarm->setPalette(pal);
    ui->Alarms->setPalette(pal);
    ui->worldClocks->setPalette(pal);
    pal.setBrush(QPalette::Active, QPalette::WindowText, QMaemo5Style::standardColor("SecondaryTextColor"));
    ui->nextAlarm->setPalette(pal);
    ui->alarmDay->setPalette(pal);
    ui->timeZone->setPalette(pal);
    ui->date_landscape->setPalette(pal);
    ui->date_portrait->setPalette(pal);


    ui->Alarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_alarm"));
    ui->wrldClk_pushButton->setIcon(QIcon::fromTheme("clock_starter_worldclock"));
    ui->nwAlarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_add_alarm"));

    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("clock_starter_add_alarm"));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("clock_starter_alarm"));
    ui->listWidget->item(2)->setIcon(QIcon::fromTheme("clock_starter_worldclock"));

    ui->action_dati_ia_adjust_date_and_time->setText(_("dati_ia_adjust_date_and_time"));
    ui->action_cloc_me_menu_settings_regional->setText(_("cloc_me_menu_settings_regional"));
    ui->action_cloc_alarm_settings_title->setText(_("cloc_alarm_settings_title"));
    // ui->action_sfil_va_number_of_objects_images->setText(ImageText);
    ui->action_disp_seconds->setText(SecondsText);
    // if ( BackgroundImg )
    // 	ui->action_sfil_va_number_of_objects_images->setChecked(true);
    if ( SecondsAdded )
	ui->action_disp_seconds->setChecked(true);

    MainDelegate *delegate = new MainDelegate(ui->listWidget);
    ui->listWidget->setItemDelegate(delegate);

    QFont f;
    f.setPointSize(48);
    ui->timeButton_landscape->setFont(f);
    f.setPointSize(18);
    ui->date_landscape->setFont(f);
    f.setPointSize(48);
    ui->timeButton_portrait->setFont(f);
    f.setPointSize(18);
    ui->date_portrait->setFont(f);

    f.setPointSize(13);
    ui->newAlarm->setFont(f);
    ui->Alarms->setFont(f);
    ui->worldClocks->setFont(f);
    ui->nextAlarm->setFont(f);
    ui->alarmDay->setFont(f);
    ui->timeZone->setFont(f);

    ui->listWidget->item(0)->setData(Qt::UserRole, _("clock_ti_new_alarm"));
    ui->listWidget->item(1)->setData(Qt::UserRole, _("cloc_ti_alarms"));
    ui->listWidget->item(2)->setData(Qt::UserRole, _("cloc_ti_world_clocks"));

    ui->newAlarm->setText(_("clock_ti_new_alarm"));
    ui->Alarms->setText(_("cloc_ti_alarms"));
    ui->worldClocks->setText(_("cloc_ti_world_clocks"));

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

    sw = new AlarmList(this);
    loadAlarm();
    ww = new World(this);
    loadWorld();

    // get current time format
    getAMPM();
    updateTime();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start(1000);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::orientationChanged()
{
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height())
    {
        ui->widget->hide();
        ui->widget_2->show();
    } else {
        ui->widget_2->hide();
        ui->widget->show();
    }
}

void MainWindow::getAMPM()
{
       // get the current time format
       GConfItem *HH24 = new GConfItem("/apps/clock/time-format");
       HH24true = HH24->value().toBool();
}

void MainWindow::updateTime()
{
       QString CurrTime;
       if (HH24true)
          CurrTime = formatHildonDate(QDateTime::currentDateTime(), hildon24hFormat);
       else
       {
          QString HH = formatHildonDate(QDateTime::currentDateTime(), hildonHHFormat);
          if ( HH.toInt() > 11 )
               CurrTime = formatHildonDate(QDateTime::currentDateTime(), hildon12hPMFormat);
          else
               CurrTime = formatHildonDate(QDateTime::currentDateTime(), hildon12hAMFormat);
       }
	
       // due to QTBUG-16136 12h/24h change is not updated in QLocale during application run
       // so we look for ourselves for time-format at the proper moments, and change it accordingly
       if ( SecondsAdded ) {
          QString secs = QTime::currentTime().toString( ":ss" );
          QRegExp TimeFormat12h( "\\D$" );
          // Non-digit last character, should be 12hr clock 
          if (TimeFormat12h.indexIn(CurrTime) != -1 ) {
                    QRegExp TimeMinutes( "\\d{2}\\s" );
                    TimeMinutes.indexIn(CurrTime);
                    QString mins = TimeMinutes.cap(0).remove(QRegExp("\\s+$"));
                    CurrTime.replace(TimeMinutes, mins + secs + " ");
                    ui->timeButton_landscape->setText(CurrTime);
          }
          else
                    ui->timeButton_landscape->setText(CurrTime + secs);
       }
       else
            ui->timeButton_landscape->setText(CurrTime);
   

    ui->timeButton_portrait->setText( ui->timeButton_landscape->text() );

    QDate fecha = QDate::currentDate();
    ui->date_landscape->setText( fecha.toString(Qt::DefaultLocaleLongDate) );
    ui->date_portrait->setText( ui->date_landscape->text() );

    ww->updateClocks();
}

void MainWindow::on_Alarm_pushButton_pressed()
{
    ui->Alarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_alarm_pressed"));
}

void MainWindow::on_Alarm_pushButton_released()
{
    ui->Alarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_alarm"));
    sw->exec();
    sw->loadAlarms();
    loadAlarm();
}

void MainWindow::on_wrldClk_pushButton_pressed()
{
    ui->wrldClk_pushButton->setIcon(QIcon::fromTheme("clock_starter_worldclock_pressed"));
}

void MainWindow::on_wrldClk_pushButton_released()
{
    ui->wrldClk_pushButton->setIcon(QIcon::fromTheme("clock_starter_worldclock"));
    ww->exec();
    loadWorld();
    // extra for refresh
    // get time_format
    getAMPM();
    sw->loadAlarms();
    loadAlarm();
    ww->loadCurrent();
}

void MainWindow::on_nwAlarm_pushButton_pressed()
{
    ui->nwAlarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_add_alarm_pressed"));
}

void MainWindow::on_nwAlarm_pushButton_released()
{
    ui->nwAlarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_add_alarm"));
    NewAlarm *al = new NewAlarm(this,false,"","","0",true,0);
    al->exec();
    delete al;
    sw->loadAlarms();
    QApplication::processEvents();
    sw->loadAlarms();
    loadAlarm();
}

void MainWindow::on_timeButton_landscape_released()
{
    osso_context_t *osso;
    osso = osso_initialize("worldclock", "", TRUE, NULL);
    osso_cp_plugin_execute(osso, "libcpdatetime.so", this, TRUE);
    // get time_format
    getAMPM();
    // refresh local time
    ww->loadCurrent();
    // refresh alarm
    sw->loadAlarms();
    loadAlarm();
    loadWorld();
}

void MainWindow::on_timeButton_portrait_released()
{
    osso_context_t *osso;
    osso = osso_initialize("worldclock", "", TRUE, NULL);
    osso_cp_plugin_execute(osso, "libcpdatetime.so", this, TRUE);
    // get time_format
    getAMPM();
    // refresh local time
    ww->loadCurrent();
    // refresh current timezone
    loadWorld();
    // refresh alarm
    sw->loadAlarms();
    loadAlarm();
    loadWorld();
}

void MainWindow::on_listWidget_itemActivated(QListWidgetItem*)
{
    int sel =  ui->listWidget->currentRow();
    ui->listWidget->clearSelection();

    if ( sel == 0 )
        on_nwAlarm_pushButton_released();
    else if ( sel == 1 )
        on_Alarm_pushButton_released();
    else if ( sel == 2 )
        on_wrldClk_pushButton_released();

}

void MainWindow::loadAlarm()
{
    ui->nextAlarm->setText(sw->line1);
    ui->alarmDay->setText(sw->line2);
    ui->listWidget->item(1)->setData(Qt::UserRole+2, sw->line1);
    ui->listWidget->item(1)->setData(Qt::UserRole+3, sw->line2);
}

void MainWindow::loadWorld()
{
    QString text = _("cloc_ti_start_gmt");
    ui->timeZone->setText( text.remove("%s") + ww->line1);
    ui->listWidget->item(2)->setData(Qt::UserRole+2, text.remove("%s") + ww->line1 );
}

void MainWindow::on_action_cloc_me_menu_settings_regional_triggered()
{
    osso_context_t *osso;
    osso = osso_initialize("worldclock", "", TRUE, NULL);
    osso_cp_plugin_execute(osso, "libcplanguageregional.so", this, TRUE);
}

void MainWindow::on_action_dati_ia_adjust_date_and_time_triggered()
{
    osso_context_t *osso;
    osso = osso_initialize("worldclock", "", TRUE, NULL);
    osso_cp_plugin_execute(osso, "libcpdatetime.so", this, TRUE);
    // get time_format
    getAMPM();
    // refresh local time
    ww->loadCurrent();
    // refresh current timezone
    loadWorld();
    // refresh alarm
    sw->loadAlarms();
    loadAlarm();
}

void MainWindow::on_action_cloc_alarm_settings_title_triggered()
{
    AlSettings *as = new AlSettings(this);
    as->show();
}

void MainWindow::on_action_sfil_va_number_of_objects_images_triggered()
{
    QSettings settings("worldclock", "worldclock");
    if ( ui->action_sfil_va_number_of_objects_images->isChecked() )
        settings.setValue("Background", "true");
    else
        settings.setValue("Background", "false");
    settings.sync();
}

void MainWindow::on_action_disp_seconds_triggered()
{
    QSettings settings("worldclock", "worldclock");
    if ( ui->action_disp_seconds->isChecked() ) {
        settings.setValue("SecondsAdded", "true");
	SecondsAdded = true;
    }
    else {
        settings.setValue("SecondsAdded", "false");
	SecondsAdded = false;
    }
    settings.sync();
}

