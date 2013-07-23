#ifndef NEWALARM_H
#define NEWALARM_H

#include <QDialog>
#include <QDateTime>
#include <QAbstractButton>

namespace Ui {
    class NewAlarm;
}

class NewAlarm : public QDialog
{
    Q_OBJECT

public:
    explicit NewAlarm(QWidget *parent = 0, bool edit = false,
                      QString Aname = "", QString Atime = "", QString Adays = "",
                      bool Acheck = false, long Acook = 0, bool show = true);
    ~NewAlarm();
    bool isEditing;
    long realcookie;
    bool enabled;
    QString name, time, days;
    int deleted;
    bool showed;
    QString longdate(QString data);

private:
    Ui::NewAlarm *ui;

public slots:
    void removeAlarm(long cookie);
    void addAlarm();

private slots:
    void on_buttonBox_2_clicked(QAbstractButton* button);
    void orientationChanged();
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_time_pushButton_pressed();
    void on_date_pushButton_pressed();
    void on_repeat_pushButton_pressed();
};

#endif // NEWALARM_H
