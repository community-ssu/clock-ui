#ifndef TDIALOG_H
#define TDIALOG_H

#include <QListWidgetItem>
#include <QAbstractButton>
#include <QDialog>

namespace Ui {
    class TDialog;
}

class TDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TDialog(QWidget *parent = 0);
    ~TDialog();
    QString selected;

private:
    Ui::TDialog *ui;

private slots:
    void on_buttonBox_2_clicked(QAbstractButton* button);
    void on_buttonBox_clicked(QAbstractButton* button);
    void on_listWidget_itemActivated(QListWidgetItem* item);
    void orientationChanged();

};

#endif // TDIALOG_H
