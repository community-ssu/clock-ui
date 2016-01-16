#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qalarmdialog.h"
#include "qpushlabel.h"
#include "qalarmpushbutton.h"
#include "qalarmlabel.h"

#include "world.h"
#include <time.h>
#include <alarm_dbus.h>
#include <libalarm.h>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QLayout>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.nokia.worldclock")

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    World * ww;
    bool showSeconds;

    void openplugin(const QByteArray &plugin);
    QString formatTimeDateMarkup(time_t tick) const;

public Q_SLOTS:
#ifdef Q_WS_MAEMO_5
    Q_SCRIPTABLE void top_application();
#endif

private:
    Ui::MainWindow *ui;
    QAlarmDialog *alarmDialog;

    QVBoxLayout *centralLayout;
    QPushLabel *timeDateLabel;

    QLayout *buttonsLayout;
    QAlarmPushButton *newAlarmButton;
    QAlarmPushButton *alarmsButton;
    QAlarmPushButton *worldclocksButton;
    QPixmap backgroundImage;

    QTimer *timer;

private Q_SLOTS:
    void on_action_cloc_alarm_settings_title_triggered();
    void on_action_cloc_me_menu_settings_regional_triggered();
    void on_action_dati_ia_adjust_date_and_time_triggered();
    void on_action_sfil_va_number_of_objects_images_triggered();
    void on_action_disp_seconds_triggered();
    void updateTime();
    void nextAlarmChanged(const QStringList &date);
    void loadWorld();

    void timeDateLabelClicked();

    void newAlarmButtonClicked();
    void alarmsButtonClicked();
    void worldclocksButtonClicked();

    void orientationChanged();
};

#endif // MAINWINDOW_H
