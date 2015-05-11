#ifndef QALARMDAYSVALUEBUTTON_H
#define QALARMDAYSVALUEBUTTON_H

#include <qalarmvaluebutton.h>

#include <QDialog>
#include <QTreeView>
#include <QStandardItemModel>
#include <QMaemo5AbstractPickSelector>
#include <QDialogButtonBox>

#include <libalarm.h>

#include "utils.h"

class QMaemo5DaysPickSelector : public QMaemo5AbstractPickSelector
{
    Q_OBJECT

public:
    explicit QMaemo5DaysPickSelector(QObject *parent = 0);
    ~QMaemo5DaysPickSelector();
    QWidget *widget(QWidget *parent);

    QString currentValueText() const
    {
        return daysFromWday(days()).join(",");
    }

    void setDays(uint32_t days)
    {
        /* Sun, Mon, ... */
        days &= ALARM_RECUR_WDAY_ALL;
        d = (days >> 1) | ((days & 1) << 6);
        updateSelection();
    }

    uint32_t days() const
    {
        return ((d << 1) | (d >> 6)) & ALARM_RECUR_WDAY_ALL;
    }

private Q_SLOTS:
    void viewClicked(const QModelIndex &index);

private:
    uint32_t d;
    QTreeView *view;
    QStandardItemModel *model;
    void appendDay(const QString &name);
    void checkRow(int row, bool check);
    void updateSelection();
    friend class DaysPickerDialog;
};

class QAlarmDaysValueButton : public QAlarmValueButton
{
    Q_OBJECT

public:
    explicit QAlarmDaysValueButton(QWidget *parent = 0);
    void setDays(uint32_t days)
    {
        selector->setDays(days);
    }

    uint32_t days() const
    {
        return selector->days();
    }

Q_SIGNALS:
    void selected(uint32_t) const;

private Q_SLOTS:
    void daysSelected(const QString &) const
    {
        emit selected(selector->days());
    }

protected:
    QMaemo5DaysPickSelector *selector;
    virtual QString valueText() const;
};

class DaysPickerDialog : public QDialog
{
    Q_OBJECT
public:
    DaysPickerDialog(QMaemo5DaysPickSelector *picker, QWidget *parent);
    ~DaysPickerDialog();

protected:
    void paintEvent(QPaintEvent *e);

private Q_SLOTS:
    void accepted();
    void orientationChanged();

private:
    QMaemo5DaysPickSelector *p;
    QDialogButtonBox *box;
};
#endif // QALARMDAYSVALUEBUTTON_H
