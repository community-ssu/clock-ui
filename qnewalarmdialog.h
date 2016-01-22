#ifndef QNEWALARMDIALOG_H
#define QNEWALARMDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QAbstractButton>
#include <QTimer>

#include <stdint.h>

namespace Ui {
    class QNewAlarmDialog;
}

class QNewAlarmDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QNewAlarmDialog(QWidget *parent = 0, bool edit = false,
                      QString Aname = "", const QTime &time = QTime(),
                      uint32_t wday = 0,
                      bool Acheck = false, long Acook = 0, bool show = true);
    ~QNewAlarmDialog();
    bool isEditing;
    long realcookie;
    bool enabled;
    QString name;
    QTime time;
    uint32_t wday;
    int deleted;
    bool showed;

private:
    Ui::QNewAlarmDialog *ui;

public Q_SLOTS:
    void removeAlarm(long cookie);
    void addAlarm();
protected Q_SLOTS:
    void timeout();

private slots:
    void on_portraitButtonBox_clicked(QAbstractButton* button);
    void orientationChanged();
    void on_landscapeButtonBox_clicked(QAbstractButton* button);
    void timeChanged(const QTime &time);
    void dateChanged(const QDate &date);
    void on_daysButton_selected(uint32_t days);
    void on_soundButton_pressed();
protected:
    QTimer timer;
};

#endif // QNEWALARMDIALOG_H
