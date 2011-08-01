#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QDialog>
#include <time.h>
#include <alarmd/alarm_dbus.h>
#include <alarmd/libalarm.h>
#include <QDateTime>
#include <QListWidgetItem>
#include <QTreeWidgetItem>

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
    void loadAlarms();

private:
    Ui::AlarmList *ui;

private slots:
    void on_treeWidget_itemActivated(QTreeWidgetItem* item, int column);
    void orientationChanged();
    //void on_listWidget_itemActivated(QListWidgetItem* item);
    void on_pushButton_pressed();
};

#endif // ALARMLIST_H
