#ifndef DIALOG3_H
#define DIALOG3_H

#include <QDialog>
#include <QKeyEvent>

namespace Ui {
    class Dialog3;
}

class Dialog3 : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog3(QWidget *parent = 0);
    ~Dialog3();

private:
    Ui::Dialog3 *ui;

protected:
    virtual void keyReleaseEvent(QKeyEvent* k);


private slots:
    void on_search_textChanged(QString );
    void on_pushButton_pressed();
};

#endif // DIALOG3_H
