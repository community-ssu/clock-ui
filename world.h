#ifndef WORLD_H
#define WORLD_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "citydetail.h"

namespace Ui {
    class World;
}

class World : public QDialog
{
    Q_OBJECT

public:
    explicit World(QWidget *parent = 0);
    ~World();
    QString line1;
    QString longdate(QString data);
    bool HH24true;
    CityDetail * c_details;

public slots:
    void updateClocks();
    void loadCurrent();
    void getAMPM();

private:
    Ui::World *ui;
    int curTime;

private slots:
    void on_treeWidget_customContextMenuRequested(QPoint pos);
    void on_treeWidget_itemActivated(QTreeWidgetItem*);
    void addCity(int cityId);
    void orientationChanged();
    void on_newWorldclock_pushButton_clicked();
    void onChildClosed();
    void removeSel();
};

#endif // WORLD_H
