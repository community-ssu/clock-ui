#include "qmaemo5rotator.h"
#include "alsettings.h"
#include "ui_alsettings.h"
#include "osso-intl.h"
#include "gconfitem.h"
#include "alarmd/libalarm.h"
#include "mdialog.h"
#include "tdialog.h"

AlSettings::AlSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlSettings)
{
    ui->setupUi(this);

    this->setWindowTitle(_("cloc_alarm_settings_title"));

    ui->pushButton->setStatusTip(_("dati_fi_alarm_tone"));
    ui->pushButton_2->setStatusTip(_("dati_fi_snooze_time"));

    GConfItem *item = new GConfItem("/apps/clock/alarm-tone");

    if ( item->value().toString() == "/usr/share/sounds/ui-clock_alarm_default.aac" )
        ui->pushButton->setValueText(_("cloc_fi_set_alarm_tone1"));
    else if ( item->value().toString() == "/usr/share/sounds/ui-clock_alarm_2.aac" )
        ui->pushButton->setValueText(_("cloc_fi_set_alarm_tone2"));
    else if ( item->value().toString() == "/usr/share/sounds/ui-clock_alarm_3.aac" )
        ui->pushButton->setValueText(_("cloc_fi_set_alarm_tone3"));
    else
        ui->pushButton->setValueText( QFileInfo(item->value().toString()).baseName() );

    int val = alarmd_get_default_snooze();
    ui->pushButton_2->setWhatsThis(QString::number(val));
    val = val/60;

    QString tmp = _("cloc_va_diff_hours_mins");
    tmp.replace("%s %d", QString::number(val));
    ui->pushButton_2->setValueText(tmp);

    ui->pushButton->setWhatsThis("alarm");
    ui->pushButton_2->setWhatsThis("alarm");

    intl("osso-connectivity-ui");
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(_("conn_bd_receive_ok"));
    ui->buttonBox_2->button(QDialogButtonBox::Save)->setText(_("conn_bd_receive_ok"));
    intl("osso-clock");


    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

}

AlSettings::~AlSettings()
{
    delete ui;
}

void AlSettings::orientationChanged()
{
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        ui->buttonBox->hide();
        ui->buttonBox_2->show();
        this->setMinimumHeight(250);
        this->setMaximumHeight(250);
        ui->gridLayout->setHorizontalSpacing(0);
    } else {
        ui->buttonBox_2->hide();
        ui->buttonBox->show();
        this->setMinimumHeight(180);
        this->setMaximumHeight(180);
        ui->gridLayout->setHorizontalSpacing(6);
    }
}

void AlSettings::on_pushButton_2_pressed()
{
    MDialog *hw = new MDialog(this);
    hw->exec();
    if ( hw->selected > -1 )
    {
        int val;

        if ( hw->selected == 300 )
            val = 5;
        else if ( hw->selected == 600 )
            val = 10;
        else if ( hw->selected == 1200 )
            val = 20;
        QString tmp = _("cloc_va_diff_hours_mins");
        tmp.replace("%s %d", QString::number(val));
        ui->pushButton_2->setValueText(tmp);
        ui->pushButton_2->setStatusTip(QString::number(hw->selected));
    }
    delete hw;

}

void AlSettings::on_pushButton_pressed()
{
    TDialog *hw = new TDialog(this);
    hw->exec();
    if ( hw->selected != "nothing" )
    {
        QString tmp = hw->selected;

        if ( tmp == "/usr/share/sounds/ui-clock_alarm_default.aac" )
            ui->pushButton->setValueText(_("cloc_fi_set_alarm_tone1"));
        else if ( tmp == "/usr/share/sounds/ui-clock_alarm_2.aac" )
            ui->pushButton->setValueText(_("cloc_fi_set_alarm_tone2"));
        else if ( tmp == "/usr/share/sounds/ui-clock_alarm_3.aac" )
            ui->pushButton->setValueText(_("cloc_fi_set_alarm_tone3"));
        else
            ui->pushButton->setValueText(tmp);

    }

    delete hw;
}

void AlSettings::on_buttonBox_clicked(QAbstractButton*)
{
    qDebug() << ui->pushButton->statusTip();
    qDebug() << ui->pushButton_2->statusTip();
    int val = ui->pushButton_2->statusTip().toInt();
    alarmd_set_default_snooze(val);

    QString text;
    if ( ui->pushButton->valueText() == _("cloc_fi_set_alarm_tone1") )
        text = "/usr/share/sounds/ui-clock_alarm_default.aac";
    else if ( ui->pushButton->valueText() == _("cloc_fi_set_alarm_tone2") )
        text = "/usr/share/sounds/ui-clock_alarm_2.aac";
    else if ( ui->pushButton->valueText() == _("cloc_fi_set_alarm_tone3") )
        text = "/usr/share/sounds/ui-clock_alarm_3.aac";
    else
    {
        text = ui->pushButton->valueText();
        GConfItem *item = new GConfItem("/apps/clock/alarm-custom");
        item->set(text);
    }

    GConfItem *item2 = new GConfItem("/apps/clock/alarm-tone");
    item2->set(text);


    this->accept();
}

void AlSettings::on_buttonBox_2_clicked(QAbstractButton* item)
{
    on_buttonBox_clicked(item);
}
