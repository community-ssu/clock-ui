#ifndef QALARMSNOOZEVALUEBUTTON_H
#define QALARMSNOOZEVALUEBUTTON_H

#include <QStandardItemModel>
#include <QMaemo5ListPickSelector>

#include "qalarmvaluebutton.h"

class QAlarmSnoozeListPickSelector : public QMaemo5ListPickSelector
{
    Q_OBJECT
public:
    explicit QAlarmSnoozeListPickSelector(QObject *parent = 0) :
        QMaemo5ListPickSelector(parent)
    {
    }
    QWidget *widget(QWidget *parent);
};

class QAlarmSnoozeValueButton : public QAlarmValueButton
{
    Q_OBJECT
public:
    explicit QAlarmSnoozeValueButton(QWidget *parent = 0);
    void setSnooze(int minutes);
    int getSnooze();

Q_SIGNALS:
    void selected(int snooze);

public Q_SLOTS:
    void snoozeSelected(QString);

protected:
    QAlarmSnoozeListPickSelector *selector;
    QStandardItemModel model;
};
#endif // QALARMSNOOZEVALUEBUTTON_H
