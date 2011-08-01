#include "qmaemo5rotator.h"
#include "dialog2.h"
#include "ui_dialog2.h"
#include "osso-intl.h"

Dialog2::Dialog2(QWidget *parent, int hh, int mm, bool ampm, bool am) :
    QDialog(parent),
    ui(new Ui::Dialog2)
{
    ui->setupUi(this);
    this->setWindowTitle(_("cloc_fi_time"));

    if ( ampm )
    {
        ui->listWidget_3->show();
        QListWidgetItem *item1 = new QListWidgetItem(ui->listWidget);
        item1->setText(QString::number(12));
        item1->setTextAlignment(Qt::AlignCenter);
        ui->listWidget->addItem(item1);
        for (int i=0; i<11; ++i)
        {
            item1 = new QListWidgetItem(ui->listWidget);
            item1->setText(QString::number(i+1));
            item1->setTextAlignment(Qt::AlignCenter);
            ui->listWidget->addItem(item1);
        }

    }
    else
    {
        ui->listWidget_3->hide();
        for (int i=0; i<24; ++i)
        {
            QListWidgetItem *item1 = new QListWidgetItem(ui->listWidget);
            QString num = QString::number(i);
            item1->setText(num);
            if ( num.length() == 1 )
                num = "0" + num;
            item1->setText(num);
            item1->setTextAlignment(Qt::AlignCenter);
            ui->listWidget->addItem(item1);
        }
    }


    for (int i=0; i<60; ++i)
    {
        QListWidgetItem *item2 = new QListWidgetItem(ui->listWidget_2);
        QString num = QString::number(i);
        item2->setText(num);
        if ( num.length() == 1 )
            num = "0" + num;
        item2->setText(num);
        item2->setTextAlignment(Qt::AlignCenter);
        ui->listWidget_2->addItem(item2);
    }


    if ( am )
        ui->listWidget_3->setCurrentRow(0);
    else
        ui->listWidget_3->setCurrentRow(1);

    intl("osso-connectivity-ui");
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(_("conn_iaps_bd_done"));
    ui->buttonBox_2->button(QDialogButtonBox::Save)->setText(_("conn_iaps_bd_done"));
    intl("osso-clock");

    if ( (ampm) && (hh==12) )
        hh = 0;



    ui->listWidget->scrollToItem(ui->listWidget->item(hh));
    ui->listWidget->item(hh)->setSelected(true);
    ui->listWidget->setCurrentRow(hh);

    ui->listWidget_2->scrollToItem(ui->listWidget_2->item(mm));
    ui->listWidget_2->item(mm)->setSelected(true);
    ui->listWidget_2->setCurrentRow(mm);

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

}

Dialog2::~Dialog2()
{
    delete ui;
}

void Dialog2::orientationChanged()
{
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        ui->buttonBox->hide();
        ui->buttonBox_2->show();
        this->setMinimumHeight(680);
        this->setMaximumHeight(680);
    } else {
        ui->buttonBox_2->hide();
        ui->buttonBox->show();
        this->setMinimumHeight(350);
        this->setMaximumHeight(350);
    }
}

void Dialog2::on_buttonBox_accepted()
{
    res1 = ui->listWidget->currentRow();
    res2 = ui->listWidget_2->currentRow();
    if ( ui->listWidget_3->currentRow() == 0 )
        isam = true;
    else
        isam = false;
    this->accept();
}

void Dialog2::on_buttonBox_2_accepted()
{
    on_buttonBox_accepted();
}
