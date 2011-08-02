#ifndef DIALOG_H
#define DIALOG_H

//#include <QDialog>
//#include <QListWidgetItem>
#include <QtMaemo5>

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0, QString name = "", QString list = "");
    ~Dialog();
    QString name;
    QStringList lista;
    QStringList result;

private:
    Ui::Dialog *ui;

private slots:
    void on_buttonBox_2_accepted();
    void orientationChanged();
    void on_buttonBox_accepted();
    void on_listWidget_itemSelectionChanged();
    void on_listWidget_itemActivated(QListWidgetItem* item);

};

#endif // DIALOG_H
