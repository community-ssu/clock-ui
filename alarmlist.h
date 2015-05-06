#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QDialog>
#include <time.h>
#include <alarm_dbus.h>
#include <libalarm.h>
#include <QDateTime>
#include <QListWidgetItem>
#include <QTreeWidgetItem>
#include <QtDBus/QDBusInterface>
#include <QStandardItemModel>

namespace Ui {
    class AlarmList;
}

class AlarmList : public QDialog
{
    Q_OBJECT

public:
    explicit AlarmList(QWidget *parent = 0);
    ~AlarmList();
    QString line1, line2;

public slots:
    void addAlarms();

private:
    Ui::AlarmList *ui;
    QStandardItemModel *alarmModel;
    QIcon iconAlarmOn;
    QIcon iconAlarmOff;
    QBrush secondaryColor;
    QFont bigFont;
    QFont smallFont;

    time_t addAlarm(cookie_t cookie);
    QStandardItem *alarmCheckboxItem(cookie_t cookie, const alarm_event_t *ae);
    QStandardItem *alarmTimeItem(const alarm_event_t *ae);
    QStandardItem *alarmTitleItem(const alarm_event_t *ae);
    QStandardItem *alarmDaysItem(const alarm_event_t *ae);

private slots:
    void on_newAlarm_clicked();
    void treeViewSelectedRow(const QModelIndex & modelIndex);
};

#endif // ALARMLIST_H
