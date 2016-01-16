#ifndef ALSETTINGS_H
#define ALSETTINGS_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
    class AlSettings;
}

class AlSettings : public QDialog
{
    Q_OBJECT

public:
    explicit AlSettings(QWidget *parent = 0);
    ~AlSettings();

private:
    Ui::AlSettings *ui;

private slots:
    void setConf(QString key = "", QString val = "", QString type = "");
    void on_buttonBox_2_clicked(QAbstractButton* button);
    void on_buttonBox_clicked(QAbstractButton* button);
    void orientationChanged();
    void on_soundButton_pressed();
    void on_snooze_pushButton_pressed();

};

#endif // ALSETTINGS_H
