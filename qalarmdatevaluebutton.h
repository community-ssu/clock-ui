#ifndef QALARMDATEVALUEBUTTON_H
#define QALARMDATEVALUEBUTTON_H

#include "qalarmvaluebutton.h"
#include <QMaemo5DatePickSelector>

class QAlarmDateValueButton : public QAlarmValueButton
{
    Q_OBJECT
public:
    explicit QAlarmDateValueButton(QWidget *parent = 0);
    QDate currentDate() const {
        return selector->currentDate();
    }
    void setCurrentDate(const QDate &date) {
        selector->blockSignals(true);
        selector->setCurrentDate(date);
        selector->blockSignals(false);
    }
    void setCurrentDate(time_t tick) {
        selector->blockSignals(true);
        selector->setCurrentDate(QDateTime::fromTime_t(tick).date());
        selector->blockSignals(false);
    }

Q_SIGNALS:
    void selected(const QDate &date);

public Q_SLOTS:
    void dateSelected(QString);

protected:
    QMaemo5DatePickSelector *selector;
    virtual QString valueText() const;
};

#endif // QALARMDATEVALUEBUTTON_H
