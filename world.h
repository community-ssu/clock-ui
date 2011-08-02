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

private:
    Ui::World *ui;

private slots:
    void on_treeWidget_itemActivated(QTreeWidgetItem* item, int column);
    void loadCurrent();
    void orientationChanged();
    void on_pushButton_pressed();
};

#endif // WORLD_H
