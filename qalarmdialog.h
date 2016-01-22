#ifndef ALARMLIST_H
#define ALARMLIST_H

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QLabel>
#include <QPushButton>
#include <QStyledItemDelegate>

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
    void addAlarm(cookie_t cookie);
    void sort();

Q_SIGNALS:
    void nextAlarmChanged(const QStringList &) const;

private:
    QLabel *label;
    QPushButton* button;
    QAlarmTreeView* view;
    QStandardItemModel *model;

    QStandardItem *alarmCheckboxItem(cookie_t cookie,
                                     const alarm_event_t *ae) const;
    void alarmTimeItem(const alarm_event_t *ae, QStandardItem *item) const;
    QStandardItem *alarmTitleItem(const alarm_event_t *ae) const;
    QStandardItem *alarmDaysItem(const alarm_event_t *ae) const;
    time_t _addAlarm(cookie_t cookie);
    void signalNextAlarm();

private Q_SLOTS:
    void buttonClicked();
    void viewClicked(const QModelIndex & modelIndex);
    void getNextAlarm() const;
};

#endif // ALARMLIST_H
