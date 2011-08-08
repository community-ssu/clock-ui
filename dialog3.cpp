#include "dialog3.h"
#include "ui_dialog3.h"
#include "cityinfo.h"
#include "clockd/libtime.h"
#include "osso-intl.h"
#include <QSettings>

Dialog3::Dialog3(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog3)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    this->setWindowTitle(_("cloc_ia_choose_time_zone"));

    ui->pushButton->setIcon(QIcon::fromTheme("general_close"));
    ui->swidget->setVisible(false);

    this->selected = "";

    int ret = 0;
    char current_tz[32];
    ret = time_get_timezone(&current_tz[0],32);
    QString defTZ = QString::fromUtf8(current_tz);
    ui->listWidget->clear();
    Cityinfo **cities = cityinfo_get_all();
    for (int i = 0; cities && cities[i]; ++i) {
        QString name = QString::fromUtf8(cityinfo_get_name(cities[i]));
        QString country = QString::fromUtf8(cityinfo_get_country(cities[i]));
        QString zone = ":" + QString::fromUtf8(cityinfo_get_zone(cities[i]));

        QString timeoffset;
        QString sign;
        int offset = time_get_utc_offset(cityinfo_get_zone(cities[i]));
        if (offset > 0)
            sign = "";
        else
            sign = "+";
        if((offset % 3600)==0)
            timeoffset = QString("GMT %3 (%1, %2)").arg(name).arg(country).arg(sign+QString::number(-offset/3600));
        else
            timeoffset = QString("GMT %3:%4 (%1, %2)").arg(name).arg(country).arg(sign+QString::number(-offset/3600)).arg("30");

        QListWidgetItem *item1 = new QListWidgetItem(ui->listWidget);
        item1->setText(timeoffset);
        item1->setStatusTip(QString::number(cities[i]->id));
        if (zone == defTZ)
            defTZ = timeoffset;
    }
    cityinfo_free_all(cities);
    ui->listWidget->sortItems();

}

Dialog3::~Dialog3()
{
    delete ui;
}

void Dialog3::on_pushButton_pressed()
{
    ui->search->setText("");
    ui->swidget->setVisible(false);
}

void Dialog3::keyReleaseEvent(QKeyEvent *k)
{
    if ( ui->search->hasFocus()==false )
    {
        ui->swidget->setVisible(true);
        ui->search->setFocus();
        if ( k->key() == 16777219 )
        {
            QString pepe = ui->search->text();
            if ( pepe.length() > 0 ) pepe.remove(pepe.length()-1,1);
            ui->search->setFocus(); ui->search->setText(pepe);
            return;
        }
        else
        {
            ui->search->setText( ui->search->text() + k->text() );
        }
    }


}

void Dialog3::on_search_textChanged(QString filter)
{
    for ( int i=0; i < ui->listWidget->count(); ++i)
    {
        if ( ui->listWidget->item(i)->text().toLower().indexOf( filter.toLower() )  == -1 )
        {
            ui->listWidget->item(i)->setHidden(true);
        }
        else
        {
            ui->listWidget->item(i)->setHidden(false);
        }
    }
}

void Dialog3::on_listWidget_itemActivated(QListWidgetItem* item)
{
    this->selected = item->statusTip();
    this->accept();
}
