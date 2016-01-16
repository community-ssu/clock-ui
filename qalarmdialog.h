#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>

#include <libalarm.h>

class QAlarmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QAlarmDialog(QWidget *parent = 0);

public Q_SLOTS:
    void addAlarms();

Q_SIGNALS:
    void nextAlarmChanged(const QStringList &);

private:
    QLabel *label;
    QPushButton* button;
    QTreeView* view;
    QStandardItemModel *model;

    time_t addAlarm(cookie_t cookie);
    QStandardItem *alarmCheckboxItem(cookie_t cookie, const alarm_event_t *ae);
    QStandardItem *alarmTimeItem(const alarm_event_t *ae);
    QStandardItem *alarmTitleItem(const alarm_event_t *ae);
    QStandardItem *alarmDaysItem(const alarm_event_t *ae);

private Q_SLOTS:
    void on_newAlarm_clicked();
    void viewClicked(const QModelIndex & modelIndex);
};

#endif // ALARMLIST_H
