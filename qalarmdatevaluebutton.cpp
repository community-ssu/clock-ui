#include "qalarmdatevaluebutton.h"
#include "osso-intl.h"
#include "utils.h"

QAlarmDateValueButton::QAlarmDateValueButton(QWidget *parent) :
    QAlarmValueButton(parent),
    selector(new QMaemo5DatePickSelector(this))
{
    setText(_("dati_fi_pr_date"));
    setPickSelector(selector);
    connect(selector, SIGNAL(selected(QString)),
            this, SLOT(dateSelected(QString)));
}

void QAlarmDateValueButton::dateSelected(QString)
{
    emit selected(currentDate());
}

QString QAlarmDateValueButton::valueText() const
{
    return formatDateTime(QDateTime(currentDate()).toTime_t(), FullDateShort);
}
