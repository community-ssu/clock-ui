#include "alsettings.h"
#include "ui_alsettings.h"
#include "osso-intl.h"
#include "gconfitem.h"
#include "libalarm.h"
#include "mdialog.h"
#include "tdialog.h"

QString musicFile;

AlSettings::AlSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AlSettings)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);

    this->setWindowTitle(_("cloc_alarm_settings_title"));

    ui->soundButton->setStatusTip(_("dati_fi_alarm_tone"));
    ui->snooze_pushButton->setStatusTip(_("dati_fi_snooze_time"));

    GConfItem *item = new GConfItem("/apps/clock/alarm-tone");

    ui->soundButton->setSoundFile(item->value().toString());

    if (ui->soundButton->isStandardSound())
        musicFile = item->value().toString();

    // get default snooze value
    int val = alarmd_get_default_snooze();

    ui->snooze_pushButton->setWhatsThis(QString::number(val));
    val = val/60;

    QString tmp = _("cloc_va_diff_hours_mins");
    tmp.replace("%s %d", QString::number(val));
    ui->snooze_pushButton->setValueText(tmp);

    ui->soundButton->setWhatsThis("alarm");
    ui->snooze_pushButton->setWhatsThis("alarm");

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

void AlSettings::on_snooze_pushButton_pressed()
{
    MDialog *hw = new MDialog(this);
    hw->exec();
    if ( hw->selected > -1 )
    {
	// convert snooze seconds to minutes
	int val = hw->selected/60;
        QString tmp = _("cloc_va_diff_hours_mins");
        tmp.replace("%s %d", QString::number(val));
        ui->snooze_pushButton->setValueText(tmp);
    }
    delete hw;

}

void AlSettings::on_soundButton_pressed()
{
    TDialog *hw = new TDialog(this);
    hw->exec();
    if ( hw->selected != "nothing" )
    {
    	musicFile = hw->selected;
        ui->soundButton->setSoundFile((musicFile));
    }
    delete hw;
}

void AlSettings::on_buttonBox_clicked(QAbstractButton*)
{
    int val = ui->snooze_pushButton->valueText().remove(QRegExp(" \\D+")).toInt();

    val = val*60;
    alarmd_set_default_snooze(val);

    QString text;
    if ( ui->soundButton->getSoundFile() == _("cloc_fi_set_alarm_tone1") )
        text = "/usr/share/sounds/ui-clock_alarm_default.aac";
    else if ( ui->soundButton->getSoundFile() == _("cloc_fi_set_alarm_tone2") )
        text = "/usr/share/sounds/ui-clock_alarm_2.aac";
    else if ( ui->soundButton->getSoundFile() == _("cloc_fi_set_alarm_tone3") )
        text = "/usr/share/sounds/ui-clock_alarm_3.aac";
    else
    {
        text = ui->soundButton->getSoundFile();
        // GConfItem changes the gconf value, but doesn't update!
        //GConfItem *item = new GConfItem("/apps/clock/alarm-custom");
        //item->set(text);
        setConf("/apps/clock/alarm-custom",text,"string");

    }

    // GConfItem changes the gconf value, but doesn't update!
    //GConfItem *item2 = new GConfItem("/apps/clock/alarm-tone");
    //item2->set(text);
    setConf("/apps/clock/alarm-tone",text,"string");

    this->accept();
}

void AlSettings::on_buttonBox_2_clicked(QAbstractButton* item)
{
    on_buttonBox_clicked(item);
}

void AlSettings::setConf(QString key, QString val, QString type)
{

    QString sended = "gconftool-2 -s "+key+" -t "+type+" \""+val+"\"";
    QByteArray ba = sended.toUtf8();
    const char *str1 = ba.data();
    system(str1);

}
