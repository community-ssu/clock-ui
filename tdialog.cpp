#include "qmaemo5rotator.h"
#include "tdialog.h"
#include "ui_tdialog.h"
#include "osso-intl.h"
#include <QDialogButtonBox>
#include "valuebutton.h"
#include "gconfitem.h"
#include "home.h"

TDialog::TDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TDialog)
{
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    ui->setupUi(this);

    selected = "nothing";

    this->setWindowTitle(_("cloc_ti_alarm_tone"));

    intl("calendar");
    ui->buttonBox->button(QDialogButtonBox::Retry)->setText(_("cal_more_events"));
    ui->buttonBox_2->button(QDialogButtonBox::Retry)->setText(_("cal_more_events"));
    intl("osso-connectivity-ui");
    ui->buttonBox->button(QDialogButtonBox::Apply)->setText(_("conn_iaps_bd_done"));
    ui->buttonBox_2->button(QDialogButtonBox::Apply)->setText(_("conn_iaps_bd_done"));
    intl("osso-clock");

    QListWidgetItem *item1;

    GConfItem *item = new GConfItem("/apps/clock/alarm-custom");
    QString text = item->value().toString();

    if ( text != "/"  )
    {
        item1 = new QListWidgetItem();
        item1->setTextAlignment(Qt::AlignCenter);
        item1->setText( QFileInfo(text).baseName() );
        item1->setWhatsThis(text);
        ui->listWidget->addItem(item1);
    }

    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(_("cloc_fi_set_alarm_tone1"));
    item1->setWhatsThis("/usr/share/sounds/ui-clock_alarm_default.aac");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(_("cloc_fi_set_alarm_tone2"));
    item1->setWhatsThis("/usr/share/sounds/ui-clock_alarm_2.aac");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(_("cloc_fi_set_alarm_tone3"));
    item1->setWhatsThis("/usr/share/sounds/ui-clock_alarm_3.aac");
    ui->listWidget->addItem(item1);

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

    ui->listWidget->setCurrentRow(0);

}

TDialog::~TDialog()
{
    delete ui;
}

void TDialog::orientationChanged()
{

    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        ui->buttonBox->hide();
        ui->buttonBox_2->show();
        this->setMinimumHeight((ui->listWidget->count()*70)+170);
        this->setMaximumHeight((ui->listWidget->count()*70)+170);
    } else {
        ui->buttonBox_2->hide();
        ui->buttonBox->show();
        this->setMinimumHeight((ui->listWidget->count()*70)+10);
        this->setMaximumHeight((ui->listWidget->count()*70)+10);
    }
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

}

void TDialog::on_listWidget_itemActivated(QListWidgetItem* item)
{
    //selected = item->whatsThis();
    //this->accept();
}

void TDialog::on_buttonBox_clicked(QAbstractButton* button)
{
    intl("calendar");
    QString tmp = _("cal_more_events");
    intl("osso-clock");
    if ( button->text() == tmp )
    {
        Home *hw = new Home(this, "/home/user/MyDocs/.sounds");
        hw->exec();
        if ( hw->selected != "" )
        {
            if ( ui->listWidget->count() == 3 )
            {
                QListWidgetItem *item1 = new QListWidgetItem();
                item1->setTextAlignment(Qt::AlignCenter);
                item1->setText( QFileInfo(hw->selected).baseName() );
                item1->setWhatsThis(hw->selected);
                ui->listWidget->insertItem(0, item1);
                orientationChanged();
                ui->listWidget->setCurrentRow(0);
            }
            else
            {
                ui->listWidget->item(0)->setText( QFileInfo(hw->selected).baseName() );
                ui->listWidget->item(0)->setWhatsThis(hw->selected);
                ui->listWidget->setCurrentRow(0);
            }
        }
        delete hw;
    }
    else
    {
        selected = ui->listWidget->currentItem()->whatsThis();
        this->accept();
    }

}

void TDialog::on_buttonBox_2_clicked(QAbstractButton* button)
{
    on_buttonBox_clicked(button);
}
