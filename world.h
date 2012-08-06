#ifndef WORLD_H
#define WORLD_H

#include <QDialog>
#include <QTreeWidgetItem>

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
    int curTime;
    QString longdate(QString data);
    bool HH24true;

public slots:
    void updateClocks();
    void loadCurrent();
    void getAMPM();

private:
    Ui::World *ui;

private slots:
    void removeSel();
    void on_treeWidget_customContextMenuRequested(QPoint pos);
    void on_treeWidget_itemActivated(QTreeWidgetItem*);
    void addCity(int cityId);
    void orientationChanged();
    void on_newWorldclock_pushButton_pressed();
};

#endif // WORLD_H
