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
#include <QDateTime>
#include <libosso.h>

//#include <hildon-extras/he-tz-chooser.h>

#include <X11/Xlib.h>
#include <gtk-2.0/gdk/gdkx.h>
#include <gtk-2.0/gtk/gtk.h>
#include <gtk-2.0/gtk/gtkwidget.h>

QSettings iconset( "/etc/hildon/theme/index.theme", QSettings::IniFormat );
QString currtheme = iconset.value("X-Hildon-Metatheme/IconTheme","hicolor").toString();

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setWindowFlags(Qt::Window);
    ui->setupUi(this);

    this->setWindowTitle(_("cloc_ap_name"));
    if ( currtheme == "default" )
        currtheme = "hicolor";
    ui->pushButton->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_alarm.png"));
    ui->pushButton_2->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_worldclock.png"));
    ui->pushButton_3->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_add_alarm.png"));

    ui->listWidget->item(0)->setData(Qt::UserRole+1, "/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_add_alarm.png");
    ui->listWidget->item(1)->setData(Qt::UserRole+1, "/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_alarm.png");
    ui->listWidget->item(2)->setData(Qt::UserRole+1, "/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_worldclock.png");

    ui->action_cloc_me_menu_settings_regional->setText(_("cloc_me_menu_settings_regional"));
    ui->action_cloc_alarm_settings_title->setText(_("cloc_alarm_settings_title"));

    MainDelegate *delegate = new MainDelegate(ui->listWidget);
    ui->listWidget->setItemDelegate(delegate);

    QFont f = ui->hour->font();
    f.setPointSize(48);
    ui->hour->setFont(f);
    f.setPointSize(18);
    ui->date->setFont(f);
    f.setPointSize(48);
    ui->hour_2->setFont(f);
    f.setPointSize(18);
    ui->date_2->setFont(f);

    f.setPointSize(13);
    ui->label->setFont(f);
    ui->label_2->setFont(f);
    ui->label_3->setFont(f);
    ui->label_4->setFont(f);
    ui->label_5->setFont(f);
    ui->label_6->setFont(f);

    ui->listWidget->item(0)->setData(Qt::UserRole, _("clock_ti_new_alarm"));
    ui->listWidget->item(1)->setData(Qt::UserRole, _("cloc_ti_alarms"));
    ui->listWidget->item(2)->setData(Qt::UserRole, _("cloc_ti_world_clocks"));

    ui->label->setText(_("clock_ti_new_alarm"));
    ui->label_2->setText(_("cloc_ti_alarms"));
    ui->label_3->setText(_("cloc_ti_world_clocks"));

    QSettings cs( "/etc/hildon/theme/colors.config", QSettings::IniFormat );
    QString bcolor = cs.value("Colors/DefaultBackgroundColor", "").toString();
    ui->widget_3->setStyleSheet("background-color: "+bcolor+";");


    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

    sw = new AlarmList(this);
    loadAlarm();
    ww = new World(this);
    loadWorld();

    updateTime();
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));
    timer->start(10000);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawImage(this->rect(), QImage("/etc/hildon/theme/backgrounds/clock.png"));
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

void MainWindow::updateTime()
{

    //QTime tiempo = QTime::currentTime();
    ui->hour->setText( QLocale::system().toString(QTime::currentTime(), QLocale::ShortFormat) );
    ui->hour_2->setText( ui->hour->text() );

    QDate fecha = QDate::currentDate();
    ui->date->setText( fecha.toString(Qt::DefaultLocaleLongDate) );
    ui->date_2->setText( ui->date->text() );

    ww->updateClocks();
}

void MainWindow::on_pushButton_pressed()
{
    ui->pushButton->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_alarm_pressed.png"));
}

void MainWindow::on_pushButton_released()
{
    ui->pushButton->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_alarm.png"));
    sw->exec();
    sw->loadAlarms();
    loadAlarm();
}

void MainWindow::on_pushButton_2_pressed()
{
    ui->pushButton_2->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_worldclock_pressed.png"));

    /*osso_context_t *osso;
    osso = osso_initialize("worldclock", "", TRUE, NULL);

    g_object_set(G_OBJECT(osso),"maincontext-pointer", osso, this);

    osso_return_t * res;
    int r = osso_cp_plugin_execute(osso, "libcpdatetime.so",
                            this, TRUE);*/


    //QAbstractEventDispatcher *ed = new QAbstractEventDispatcher(this);

    /*Citytime *home = clock_get_home_location ();
    HildonTimeZoneChooser* tc = hildon_time_zone_chooser_new();
    hildon_time_zone_chooser_set_city (tc, home->city);
    g_object_set(G_OBJECT(tc),"maincontext-pointer", tc, this);
    if ( hildon_time_zone_chooser_run(tc) == 1 )
    {
        Cityinfo* ci = hildon_time_zone_chooser_get_city(tc);
        qDebug() << ci->name << ci->country << ci->code << ci->zone;
        cityinfo_free(ci);
    }
    clock_citytime_free (home);
    hildon_time_zone_chooser_free(tc);*/

    /*showNormal();
    raise();
    activateWindow();*/

}

void MainWindow::on_pushButton_2_released()
{
    ui->pushButton_2->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_worldclock.png"));
    ww->exec();
    loadWorld();
}

void MainWindow::on_pushButton_3_pressed()
{
    ui->pushButton_3->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_add_alarm_pressed.png"));
}

void MainWindow::on_pushButton_3_released()
{
    ui->pushButton_3->setIcon(QIcon("/usr/share/icons/" + currtheme + "/164x164/hildon/clock_starter_add_alarm.png"));
    NewAlarm *al = new NewAlarm(this,false,"","","0",true,0);
    al->exec();
    delete al;
    sw->loadAlarms();
    QApplication::processEvents();
    sw->loadAlarms();
    loadAlarm();
}

void MainWindow::on_listWidget_itemActivated(QListWidgetItem*)
{
    int sel =  ui->listWidget->currentRow();
    ui->listWidget->clearSelection();

    if ( sel == 0 )
        on_pushButton_3_released();
    else if ( sel == 1 )
        on_pushButton_released();
    else if ( sel == 2 )
        on_pushButton_2_released();


}

void MainWindow::loadAlarm()
{
    ui->label_4->setText(sw->line1);
    ui->label_5->setText(sw->line2);
    ui->listWidget->item(1)->setData(Qt::UserRole+2, sw->line1);
    ui->listWidget->item(1)->setData(Qt::UserRole+3, sw->line2);
}

void MainWindow::loadWorld()
{
    QString text = _("cloc_ti_start_gmt");
    ui->label_6->setText( text.remove("%s") + "(" + ww->line1 + ")");
    ui->listWidget->item(2)->setData(Qt::UserRole+2, text.remove("%s") + "(" + ww->line1 + ")");
}

void MainWindow::on_action_cloc_me_menu_settings_regional_triggered()
{
    osso_context_t *osso;
    osso = osso_initialize("worldclock", "", TRUE, NULL);
    osso_cp_plugin_execute(osso, "libcplanguageregional.so", this, TRUE);
}

void MainWindow::on_action_cloc_alarm_settings_title_triggered()
{
    AlSettings *as = new AlSettings(this);
    as->show();
}
