#include "qalarmtimevaluebutton.h"
#include "osso-intl.h"

#include <QDebug>

QAlarmTimeValueButton::QAlarmTimeValueButton(QWidget *parent) :
    QAlarmValueButton(parent),
    selector(new QMaemo5TimePickSelector(this))
{
    setText(_("cloc_fi_time"));
    setPickSelector(selector);
    connect(selector, SIGNAL(selected(QString)),
            this, SLOT(timeSelected(QString)));
}

void QAlarmTimeValueButton::timeSelected(QString s)
{
    Q_UNUSED(s);

    emit selected(currentTime());
}
