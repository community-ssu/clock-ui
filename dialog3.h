#ifndef DIALOG3_H
#define DIALOG3_H

#include <QDialog>
#include <QKeyEvent>
#include <QListWidgetItem>

namespace Ui {
    class Dialog3;
}

class Dialog3 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog3(QWidget *parent = 0);
    ~Dialog3();
    QString selected;

private:
    Ui::Dialog3 *ui;

protected:
    virtual void keyReleaseEvent(QKeyEvent* k);


private slots:
    void on_listWidget_itemActivated(QListWidgetItem* item);
    void on_search_textChanged(QString );
    void on_pushButton_pressed();
};

#endif // DIALOG3_H
