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
    this->setWindowTitle(_("cloc_ia_choose_time_zone"));

    QSettings settings( "/etc/hildon/theme/index.theme", QSettings::IniFormat );
    QString currtheme = settings.value("X-Hildon-Metatheme/IconTheme","hicolor").toString();
    ui->pushButton->setIcon( QIcon("/usr/share/icons/" + currtheme + "/48x48/hildon/general_close.png") );
    ui->swidget->setVisible(false);


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
        // some offset not rounded to hours, so check this and form QString in form
        //  City Name + Country + Time Offset
        if((offset % 3600)==0)
            timeoffset = QString("GTM %3 (%1, %2)").arg(name).arg(country).arg(sign+QString::number(-offset/3600));
        else
            timeoffset = QString("GTM %3:%4 (%1, %2)").arg(name).arg(country).arg(sign+QString::number(-offset/3600)).arg("30");
        // add new record to list (formed QString - to text field, and real offset in seconds - to User field)

        QListWidgetItem *item1 = new QListWidgetItem(ui->listWidget);
        item1->setText(timeoffset);
        //item1->setTextAlignment(Qt::AlignCenter);

        //aBox->addItem(timeoffset,-offset);
        // if current city time offset is equal to those got from system - store it's offset
        if (zone == defTZ)
            defTZ = timeoffset;
    }
    // free unneded city list
    cityinfo_free_all(cities);
    // sort the list alphabetically (by the City Name)
    //aBox->model()->sort(0);
    // loking for stored time offest in sorted list
    //int k = aBox->findText(defTZ);
    // setting list position to the city with time offset, which we get in the beginning
    //aBox->setCurrentIndex(k);
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
    //ui->search->setText( ui->search->text().toLower() );
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
