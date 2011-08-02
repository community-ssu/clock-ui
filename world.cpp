#include "qabstracteventdispatcher.h"
#include "qmaemo5rotator.h"
#include "world.h"
#include "ui_world.h"
#include "osso-intl.h"
#include "dialog3.h"
#include "filedelegate.h"
#include "cityinfo.h"
#include "clockd/libtime.h"
#include <libosso.h>
#include <QDebug>
#include <QSettings>

World::World(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::World)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setAttribute(Qt::WA_Maemo5StackedWindow);
    this->setWindowFlags(Qt::Window);

    this->setWindowTitle(_("cloc_ti_world_clocks"));

    QSettings settings( "/etc/hildon/theme/index.theme", QSettings::IniFormat );
    QString currtheme = settings.value("X-Hildon-Metatheme/IconTheme","hicolor").toString();
    ui->pushButton->setIcon(QIcon("/usr/share/icons/" + currtheme + "/48x48/hildon/general_add.png"));
    ui->pushButton->setText(_("cloc_me_new_world_clock"));

    FileDelegate *pluginDelegate = new FileDelegate(ui->treeWidget);
    ui->treeWidget->setItemDelegate(pluginDelegate);

    loadCurrent();

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

}

World::~World()
{
    delete ui;
}

void World::orientationChanged()
{
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height())
    {
        ui->treeWidget->header()->resizeSection(0,130);
        ui->treeWidget->header()->resizeSection(1,210);
        ui->treeWidget->header()->resizeSection(2,106);
    } else {
        ui->treeWidget->header()->resizeSection(0,130);
        ui->treeWidget->header()->resizeSection(1,430);
        ui->treeWidget->header()->resizeSection(2,206);
    }
    ui->treeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

}

void World::on_pushButton_pressed()
{
    Dialog3* hw = new Dialog3(this);
    hw->exec();

}

void World::loadCurrent()
{
    int ret = 0;
    char current_tz[32];
    QString timeoffset;
    ret = time_get_timezone(&current_tz[0], 32);
    if(ret>0 && ret < 32)
    {
      Cityinfo** cities = cityinfo_get_all();
      Cityinfo** cities_iter = cities;
      bool current_found = false;
      while(*cities_iter && !current_found)
      {
        char* tz = cityinfo_get_zone(*cities_iter);

        if(g_strcmp0(tz, current_tz+1)==0)
        {
          current_found = true;

          QTreeWidgetItem *pepe = new QTreeWidgetItem();

          QTime tiempo = QTime::currentTime();
          pepe->setText(0, tiempo.toString(Qt::DefaultLocaleShortDate) );
          pepe->setWhatsThis(0, "world-time");

          pepe->setText(1, _("cloc_fi_local_time") + " startdesc" +
                        cityinfo_clone(*cities_iter)->name + ", " +
                        cityinfo_clone(*cities_iter)->country  );
          pepe->setWhatsThis(1, "world-name");

          int offset = time_get_utc_offset(cityinfo_get_zone(*cities_iter));
          QString sign;
          if (offset > 0)
              sign = "";
          else
              sign = "+";
          if((offset % 3600)==0)
              timeoffset = QString("GTM %3").arg(sign+QString::number(-offset/3600));
          else
              timeoffset = QString("GTM %3:%4").arg(sign+QString::number(-offset/3600)).arg("30");

          QDate fecha = QDate::currentDate();
          pepe->setText(2, fecha.shortDayName(fecha.day()) + " "
                        + fecha.toString(Qt::DefaultLocaleShortDate)
                        + "  startdesc" + timeoffset );

          pepe->setWhatsThis(2, "world-date");

          ui->treeWidget->addTopLevelItem(pepe);

        }
        ++cities_iter;
      }
      cityinfo_free_all(cities);
    }

    line1 = timeoffset;

}

void World::on_treeWidget_itemActivated(QTreeWidgetItem* item, int )
{
    if ( item->text(1).contains(_("cloc_fi_local_time")) )
    {
        osso_context_t *osso;
        osso = osso_initialize("worldclock", "", TRUE, NULL);

        osso_return_t * res;
        if ( osso_cp_plugin_execute(osso, "libcpdatetime.so", this, TRUE) )
        loadCurrent();
    }



}
