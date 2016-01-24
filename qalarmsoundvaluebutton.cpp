#include <QFileInfo>

#include <osso-intl.h>

#include "qalarmsoundvaluebutton.h"

QAlarmSoundValueButton::QAlarmSoundValueButton(QWidget *parent) :
    QAlarmValueButton(parent),
    standard(false)
{
    setValueLayout(QMaemo5ValueButton::ValueUnderText);
    setText(_("dati_fi_alarm_tone"));
}

void QAlarmSoundValueButton::setSoundFile(const QString &file)
{
    standard = true;
    soundFile = file;

    if (file == "/usr/share/sounds/ui-clock_alarm_default.aac")
        setValueText(_("cloc_fi_set_alarm_tone1"));
    else if (file == "/usr/share/sounds/ui-clock_alarm_2.aac")
        setValueText(_("cloc_fi_set_alarm_tone2"));
    else if (file == "/usr/share/sounds/ui-clock_alarm_3.aac")
        setValueText(_("cloc_fi_set_alarm_tone3"));
    else
    {
        setValueText(QFileInfo(file).baseName());
        standard = false;
    }
}

QString QAlarmSoundValueButton::getSoundFile() const
{
    return soundFile;
}
