#ifndef DIALOG2_H
#define DIALOG2_H

#include <QDialog>
#include <QtMaemo5>

namespace Ui {
    class Dialog2;
}

class Dialog2 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog2(QWidget *parent = 0, int hh = 0, int mm = 0,
                     bool ampm = false, bool am = true,
                     QString dam = "", QString dpm = "");
    ~Dialog2();
    int res1;
    int res2;
    bool isam;

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_2_accepted();
    void orientationChanged();
	void centerView();
	void on_listWidget_clicked();
	void on_listWidget_2_clicked();

private:
    Ui::Dialog2 *ui;
};

#endif // DIALOG2_H
