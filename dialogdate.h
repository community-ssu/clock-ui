#ifndef DIALOGDATE_H
#define DIALOGDATE_H

#include <QDialog>
#include <QtMaemo5>

namespace Ui {
    class DialogDate;
}

class DialogDate : public QDialog
{
    Q_OBJECT

public:
    explicit DialogDate(QWidget *parent = 0, QString displayDay = "", QString displayMonth = "", QString displayYear = "");
    ~DialogDate();
    int dayres;
    int monthres;
    int yearres;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_2_accepted();
    void orientationChanged();
    void on_listWidget_year_itemClicked();
    void on_listWidget_month_itemClicked();
    void on_listWidget_day_itemClicked();

private:
    Ui::DialogDate *ui;
};

#endif // DIALOGDATE_H
