#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "alarmlist.h"
#include "osso-intl.h"
#include "newalarm.h"
#include "maintdelegate.h"
#include "world.h"
#include "alsettings.h"
#include "gconfitem.h"
#include <QDateTime>
#include <QMaemo5Style>
#include <QSettings>
#include <QTimer>
#include <QTextDocument>

#include <dlfcn.h>

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
bool dl_loaded = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->setupUi(this);

    this->setWindowTitle(_("cloc_ap_name"));

    // Read some variables
    // set background by default
    bool BackgroundImg = true;
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

    if (BackgroundImg)
    {
        QFileInfo bg_fileInfo("/etc/hildon/theme/backgrounds/clock.png");
        if (bg_fileInfo.exists())
        {
            QPalette pal(palette());
            setAutoFillBackground(true);
            pal.setBrush(QPalette::Window,
                         QPixmap("/etc/hildon/theme/backgrounds/clock.png"));
            setPalette(pal);
        }
    }

    // set to secondary text color
    {
        QPalette pal(palette());
        pal.setColor(QPalette::WindowText,
                     QMaemo5Style::standardColor("SecondaryTextColor"));

        ui->nextAlarm->setPalette(pal);
        ui->alarmDay->setPalette(pal);
        ui->timeZone->setPalette(pal);
        ui->date_landscape->setPalette(pal);
        ui->dateButton_portrait->setPalette(pal);
    }

    QPalette pal(palette());
    pal.setColor(QPalette::Background, QMaemo5Style::standardColor("DefaultBackgroundColor"));
    ui->widget_3->setAutoFillBackground(true);
    ui->widget_3->setPalette(pal);

    pal.setBrush(QPalette::Active, QPalette::WindowText, QMaemo5Style::standardColor("DefaultTextColor"));
    ui->widget_3->setPalette(pal);
    ui->newAlarm->setPalette(pal);
    ui->Alarms->setPalette(pal);
    ui->worldClocks->setPalette(pal);

    ui->Alarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_alarm"));
    ui->wrldClk_pushButton->setIcon(QIcon::fromTheme("clock_starter_worldclock"));
    ui->nwAlarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_add_alarm"));

    ui->listWidget->item(0)->setIcon(QIcon::fromTheme("clock_starter_add_alarm"));
    ui->listWidget->item(1)->setIcon(QIcon::fromTheme("clock_starter_alarm"));
    ui->listWidget->item(2)->setIcon(QIcon::fromTheme("clock_starter_worldclock"));

    ui->action_dati_ia_adjust_date_and_time->setText(_("dati_ia_adjust_date_and_time"));
    ui->action_cloc_me_menu_settings_regional->setText(_("cloc_me_menu_settings_regional"));
    ui->action_cloc_alarm_settings_title->setText(_("cloc_alarm_settings_title"));
    ui->action_disp_seconds->setText(SecondsText);
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
    ui->dateButton_portrait->setFont(f);

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

//    connect(ui->Alarm_pushButton, SIGNAL(clicked()), this, SLOT(Alarm_pushButton_picked()));

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
	// portrait
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
       delete HH24;
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
	
	   CurrTime = CurrTime.trimmed();
       ui->timeButton_landscape->setText("");
       ui->timeButton_portrait->setText("");
       QTextDocument Text;
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
                    CurrTime.replace(TimeMinutes, mins + secs + "<span style=font-size:25px;> ");
		    Text.setHtml("<span style=font-size:60px;>" + CurrTime + "</span></span>");
		    QPixmap pixmap(Text.size().width(), Text.size().height());
		    pixmap.fill( Qt::transparent );
		    QPainter painter( &pixmap );
		    Text.drawContents(&painter, pixmap.rect());
		    QIcon ButtonIcon(pixmap);
		    ui->timeButton_landscape->setIcon(ButtonIcon);
		    ui->timeButton_landscape->setIconSize(pixmap.rect().size());
          }
          else
	  {
		    Text.setHtml("<span style=font-size:60px;>" + CurrTime + secs + "</span>");
		    QPixmap pixmap(Text.size().width(), Text.size().height());
		    pixmap.fill( Qt::transparent );
		    QPainter painter( &pixmap );
		    Text.drawContents(&painter, pixmap.rect());
		    QIcon ButtonIcon(pixmap);
		    ui->timeButton_landscape->setIcon(ButtonIcon);
		    ui->timeButton_landscape->setIconSize(pixmap.rect().size());
	  }
       }
       else
       {
		   // no seconds added
            CurrTime.replace(" ", "<span style=font-size:25px;> ");
	    Text.setHtml("<span style=font-size:60px;>" + CurrTime + "</span>");
	    QPixmap pixmap(Text.size().width(), Text.size().height());
	    pixmap.fill( Qt::transparent );
	    QPainter painter( &pixmap );
	    Text.drawContents(&painter, pixmap.rect());
	    QIcon ButtonIcon(pixmap);
	    ui->timeButton_landscape->setIcon(ButtonIcon);
	    ui->timeButton_landscape->setIconSize(pixmap.rect().size());
       }
   
       // copy it to the same portrait button
       ui->timeButton_portrait->setIcon( ui->timeButton_landscape->icon() );
       ui->timeButton_portrait->setIconSize(ui->timeButton_landscape->iconSize());

       QDate fecha = QDate::currentDate();
       ui->date_landscape->setText( fecha.toString(Qt::DefaultLocaleLongDate) );
       ui->dateButton_portrait->setText( ui->date_landscape->text() );
       // also update the worldclockscreen clocks
       ww->updateClocks();
}

void MainWindow::on_Alarm_pushButton_pressed()
{
    ui->Alarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_alarm_pressed"));
}

void MainWindow::on_Alarm_pushButton_released()
{
    ui->Alarm_pushButton->setIcon(QIcon::fromTheme("clock_starter_alarm"));
}

void MainWindow::on_Alarm_pushButton_clicked()
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
}

void MainWindow::on_wrldClk_pushButton_clicked()
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
}

void MainWindow::on_nwAlarm_pushButton_clicked()
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

void MainWindow::on_timeButton_landscape_clicked()
{
    if (! dl_loaded ) // do not reload if already active
    {	    
	    dl_loaded = true;
	    openplugin("libcpdatetime");
	    // get time_format
	    getAMPM();
	    // refresh local time
	    ww->loadCurrent();
	    // refresh alarm
	    loadAlarm();
	    sw->loadAlarms();
	    loadWorld();
	    dl_loaded = false;
    }
}

void MainWindow::on_timeButton_portrait_clicked()
{
	on_timeButton_landscape_clicked();
}

void MainWindow::on_dateButton_portrait_clicked()
{
	on_timeButton_landscape_clicked();
}

void MainWindow::on_listWidget_itemActivated(QListWidgetItem*)
{
    int sel =  ui->listWidget->currentRow();
    ui->listWidget->clearSelection();

    if ( sel == 0 )
        on_nwAlarm_pushButton_clicked();
    else if ( sel == 1 )
        on_Alarm_pushButton_clicked();
    else if ( sel == 2 )
        on_wrldClk_pushButton_clicked();

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
    openplugin("libcplanguageregional");
}

void MainWindow::on_action_dati_ia_adjust_date_and_time_triggered()
{
    openplugin("libcpdatetime");
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

#ifdef Q_WS_MAEMO_5
void MainWindow::top_application()
{
    raise();
    activateWindow();
}
#endif

void MainWindow::openplugin(const QByteArray &plugin)
{
    void * handle;
    int (*execute)(void *, void *, int);

    /* very very ugly hack:
       second param to execute should be pointer to GtkWidget, when is NULL libcpdatetime.so not working
       because Gtk checking if this is valid GtkWidget pointer, we can pass some valid pointer...
       this allow us to open libcpdatetime.so from Qt without Gtk parent widget
     */
    int dummy = 0;

    handle = dlopen(QByteArray("/usr/lib/hildon-control-panel/") + plugin + ".so", RTLD_LAZY);
    if ( handle ) {
        execute = (int(*)(void *, void *, int))dlsym(handle, "execute");
        if ( execute ) {
            execute(NULL, &dummy, 1);
        }
        dlclose(handle);
    }
}
