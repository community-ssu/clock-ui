#ifndef HOME_H
#define HOME_H

#include <QDialog>
#include <QListWidgetItem>
#include <QMaemo5ValueButton>

namespace Ui {
    class Home;
}

class Home : public QDialog
{
    Q_OBJECT

public:
    explicit Home(QWidget *parent = 0, QString path = "");
    ~Home();
    QMaemo5ValueButton* folderbutton;
    QString selected;

public slots:
    void CargarBrowser(QString directorio);

private:
    Ui::Home *ui;

private slots:
    void on_folderButton_up_clicked();
    void on_listWidget_itemClicked(QListWidgetItem* item);
};

#endif // HOME_H
