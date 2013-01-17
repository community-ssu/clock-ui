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
    QString longdate(QString data);

public slots:
    void loadAlarms();

private:
    Ui::AlarmList *ui;

private slots:
    void on_treeWidget_itemActivated(QTreeWidgetItem* item, int column);
    void orientationChanged();
    //void on_listWidget_itemActivated(QListWidgetItem* item);
    void on_newAlarm_pressed();
};

#endif // ALARMLIST_H
