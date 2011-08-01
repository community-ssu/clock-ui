#ifndef MDIALOG_H
#define MDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
    class MDialog;
}

class MDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MDialog(QWidget *parent = 0);
    ~MDialog();
    int selected;

private:
    Ui::MDialog *ui;

private slots:
    void on_listWidget_itemActivated(QListWidgetItem* item);
};

#endif // MDIALOG_H
