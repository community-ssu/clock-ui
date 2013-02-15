#ifndef CITYDETAIL_H
#define CITYDETAIL_H

#include <QMainWindow>
#include <QAction>
#include <QMenu>
//#include <time.h>

namespace Ui {
    class CityDetail;
}

class CityDetail : public QMainWindow
{
    Q_OBJECT

public:
    explicit CityDetail(QWidget *parent = 0, QString city_country ="", QString city_code = "0", QString external_gmt = "", QString local_gmt = "", bool hh24 = true, int offset_sec = 0 );
    ~CityDetail();
    QString longdate(QString data);

private:
    Ui::CityDetail *ui;
    QTimer *timer;

private slots:
	void on_actionDelete_triggered();
        void updateInfo();
};

#endif // CITYDETAIL_H
