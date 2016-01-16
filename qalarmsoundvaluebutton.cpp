#include <QFileInfo>

#include <osso-intl.h>

#include "qalarmsoundvaluebutton.h"

QAlarmSoundValueButton::QAlarmSoundValueButton(QWidget *parent) :
    QAlarmValueButton(parent),
    standart(false)
{
    setText(_("cloc_ti_alarm_notification_title"));
}

void QAlarmSoundValueButton::setSoundFile(const QString &file)
{
    standart = true;
    if (file == "/usr/share/sounds/ui-clock_alarm_default.aac")
        setValueText(_("cloc_fi_set_alarm_tone1"));
    else if (file == "/usr/share/sounds/ui-clock_alarm_2.aac")
        setValueText(_("cloc_fi_set_alarm_tone2"));
    else if (file == "/usr/share/sounds/ui-clock_alarm_3.aac")
        setValueText(_("cloc_fi_set_alarm_tone3"));
    else
    {
        setValueText(QFileInfo(file).baseName());
        standart = false;
    }
}

QString QAlarmSoundValueButton::getSoundFile() const
{
    return valueText();
}

void QAlarmSoundValueButton::setValueText(const QString &text)
{
    QAlarmValueButton::setValueText(text);
}

