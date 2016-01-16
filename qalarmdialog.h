#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>

#include <libalarm.h>

class QAlarmTreeView : public QTreeView
{
    Q_OBJECT
public:
    explicit QAlarmTreeView(QWidget *parent = 0) :
        QTreeView(parent),
        valid(false)
    {
    }
    bool itemValid()
    {
        return valid;
    }

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    QPoint pressPos;
    bool valid;
};

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
    QAlarmTreeView* view;
    QStandardItemModel *model;

    time_t addAlarm(cookie_t cookie);
    QStandardItem *alarmCheckboxItem(cookie_t cookie, const alarm_event_t *ae);
    QStandardItem *alarmTimeItem(const alarm_event_t *ae);
    QStandardItem *alarmTitleItem(const alarm_event_t *ae);
    QStandardItem *alarmDaysItem(const alarm_event_t *ae);

private Q_SLOTS:
    void buttonClicked();
    void viewClicked(const QModelIndex & modelIndex);
};

#endif // ALARMLIST_H
