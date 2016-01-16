#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "osso-intl.h"
#include "qnewalarmdialog.h"
#include "world.h"
#include "alsettings.h"
#include "gconfitem.h"

#include <QDateTime>
#include <QMaemo5Style>
#include <QSettings>
#include <QTimer>
#include <QTextDocument>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <dlfcn.h>
#include "utils.h"

bool dl_loaded = false;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    buttonsLayout(0),
    newAlarmButton(new QAlarmPushButton(_("clock_ti_new_alarm"))),
    alarmsButton(new QAlarmPushButton(_("cloc_ti_alarms"))),
    worldclocksButton(new QAlarmPushButton(_("cloc_ti_world_clocks"))),
    backgroundImage("/etc/hildon/theme/backgrounds/clock.png")
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    ui->setupUi(this);

    setWindowTitle(_("cloc_ap_name"));

    centralLayout = new QVBoxLayout(centralWidget());
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    QHBoxLayout *hl = new QHBoxLayout;
    timeDateLabel = new QPushLabel;
    connect(timeDateLabel, SIGNAL(clicked()),
            this, SLOT(timeDateLabelClicked()));
    hl->addWidget(timeDateLabel);

    centralLayout->addLayout(hl);
    centralLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum,
                                  QSizePolicy::Expanding));

    newAlarmButton->setIcon("clock_starter_add_alarm");
    alarmsButton->setIcon("clock_starter_alarm");
    worldclocksButton->setIcon("clock_starter_worldclock");

    connect(newAlarmButton, SIGNAL(clicked()),
            this, SLOT(newAlarmButtonClicked()));
    connect(alarmsButton, SIGNAL(clicked()),
            this, SLOT(alarmsButtonClicked()));
    connect(worldclocksButton, SIGNAL(clicked()),
            this, SLOT(worldclocksButtonClicked()));

    // Read some variables
    // set background by default
    bool BackgroundImg = true;
    QSettings settings("worldclock", "worldclock");
    if (settings.contains("Background"))
        BackgroundImg = settings.value("Background").toBool();
    showSeconds = settings.value("SecondsAdded").toBool();
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

    ui->action_dati_ia_adjust_date_and_time->setText(_("dati_ia_adjust_date_and_time"));
    ui->action_cloc_me_menu_settings_regional->setText(_("cloc_me_menu_settings_regional"));
    ui->action_cloc_alarm_settings_title->setText(_("cloc_alarm_settings_title"));
    ui->action_disp_seconds->setText(SecondsText);

    if (showSeconds)
        ui->action_disp_seconds->setChecked(true);

    connect(QApplication::desktop(), SIGNAL(resized(int)),
            this, SLOT(orientationChanged()));

    orientationChanged();

    alarmDialog = new QAlarmDialog(this);
    connect(alarmDialog, SIGNAL(nextAlarmChanged(const QStringList &)),
            this, SLOT(nextAlarmChanged(const QStringList &)));

    ww = new World(this);
    loadWorld();

    // get current time format
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
    QRect geom = QApplication::desktop()->screenGeometry();

    setUpdatesEnabled(false);

    if (buttonsLayout)
    {
        buttonsLayout->removeWidget(newAlarmButton);
        buttonsLayout->removeWidget(alarmsButton);
        buttonsLayout->removeWidget(worldclocksButton);
        delete buttonsLayout;
    }

    if (geom.width() < geom.height())
    {
        /* portrait */
        setPalette(QWidget().palette());

        buttonsLayout = new QVBoxLayout;
        buttonsLayout->setSpacing(0);
        buttonsLayout->setContentsMargins(0, 0, 0, 0);

        centralLayout->addLayout(buttonsLayout);

        newAlarmButton->setLabelPosition(QAlarmPushButton::Right);
        alarmsButton->setLabelPosition(QAlarmPushButton::Right);
        worldclocksButton->setLabelPosition(QAlarmPushButton::Right);
    }
    else
    {
        buttonsLayout = new QHBoxLayout;
        buttonsLayout->setSpacing(0);

        buttonsLayout->setContentsMargins(54, 0, 54, 0);
        newAlarmButton->setLabelPosition(QAlarmPushButton::Bottom);
        alarmsButton->setLabelPosition(QAlarmPushButton::Bottom);
        worldclocksButton->setLabelPosition(QAlarmPushButton::Bottom);

        centralLayout->addLayout(buttonsLayout);

        QPalette pal(palette());
        pal.setBrush(QPalette::Window,
                     backgroundImage);
        setPalette(pal);

    }

    buttonsLayout->addWidget(newAlarmButton);
    buttonsLayout->addWidget(alarmsButton);
    buttonsLayout->addWidget(worldclocksButton);

    setUpdatesEnabled(true);
}

void MainWindow::updateTime()
{
    time_t now = QDateTime::currentDateTime().toTime_t();
    QString markup = formatTimeDateMarkup(now);

    timeDateLabel->setText(markup);

    ww->updateClocks();
}

void MainWindow::nextAlarmChanged(const QStringList &date)
{
    alarmsButton->setText(date);
}

void MainWindow::alarmsButtonClicked()
{
    alarmDialog->exec();
    alarmDialog->addAlarms();
}

void MainWindow::worldclocksButtonClicked()
{
    ww->exec();
    loadWorld();
    alarmDialog->addAlarms();
    ww->loadCurrent();
}

void MainWindow::newAlarmButtonClicked()
{
    QNewAlarmDialog *al = new QNewAlarmDialog(this, false,"",QTime(), 0,true,0);
    al->exec();
    delete al;
    alarmDialog->addAlarms();
    QApplication::processEvents();
    alarmDialog->addAlarms();
}

void MainWindow::timeDateLabelClicked()
{
    if (! dl_loaded ) // do not reload if already active
    {	    
	    dl_loaded = true;
	    openplugin("libcpdatetime");
	    // refresh local time
	    ww->loadCurrent();
	    // refresh alarm
        alarmDialog->addAlarms();
	    loadWorld();
	    dl_loaded = false;
    }
}


void MainWindow::loadWorld()
{
    QString text = _("cloc_ti_start_gmt");
    worldclocksButton->setText( QStringList() << text.remove("%s") + ww->line1);
}

void MainWindow::on_action_cloc_me_menu_settings_regional_triggered()
{
    openplugin("libcplanguageregional");
}

void MainWindow::on_action_dati_ia_adjust_date_and_time_triggered()
{
    openplugin("libcpdatetime");
    // refresh local time
    ww->loadCurrent();
    // refresh current timezone
    loadWorld();
    // refresh alarm
    alarmDialog->addAlarms();
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
    if ( ui->action_disp_seconds->isChecked() )
    {
        settings.setValue("SecondsAdded", "true");
        showSeconds = true;
    }
    else
    {
        settings.setValue("SecondsAdded", "false");
        showSeconds = false;
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

QString MainWindow::formatTimeDateMarkup(time_t tick) const
{
    QString markup;

    if (GConfItem("/apps/clock/time-format").value().toBool())
    {
        QString time;

        if (showSeconds)
        {
            char buf[256];
            size_t len = 0;
            const struct tm *t = localtime(&tick);

            len = _strftime(buf, sizeof(buf), "wdgt_va_full_24h_time", t);
            time = QString::fromUtf8(buf, len);
        }
        else
            time = formatDateTime(tick, Time);

        markup = "<p align=center style=font-size:70px;>" + time + "</p>";
    }
    else
    {
        char buf[256];
        size_t len = 0;
        const struct tm *t = localtime(&tick);
        QString tmp;

        len = _strftime(buf, sizeof(buf), "wdgt_va_12h_hours", t);
        tmp = QString::fromUtf8(buf, len) + ":";

        if (showSeconds)
            len = _strftime(buf, sizeof(buf), "wdgt_va_minutes_seconds", t);
        else
            len = _strftime(buf, sizeof(buf), "wdgt_va_minutes", t);

        tmp += QString::fromUtf8(buf, len);
        markup = "<p align=center style=font-size:70px;>" + tmp;

        if (t->tm_hour > 11)
            len = _strftime(buf, sizeof(buf), "wdgt_va_pm", t);
        else
            len = _strftime(buf, sizeof(buf), "wdgt_va_am", t);

        markup +=
                "<span style=font-size:medium;> " +
                QString::fromUtf8(buf, len);
        markup += "</span></p>";
    }

     markup +=
             "<p align=center style=font-size:medium;color:" +
             QMaemo5Style::standardColor("SecondaryTextColor").name() + ";>" +
             formatDateTime(tick, FullDateLong) +
             "</p>";

     return markup;
}
