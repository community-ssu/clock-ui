#include "mdialog.h"
#include "ui_mdialog.h"
#include "osso-intl.h"


MDialog::MDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MDialog)
{
    ui->setupUi(this);

    selected = -1;

    this->setWindowTitle(_("dati_fi_snooze_time"));

    QString tmp = _("cloc_va_diff_hours_mins");
    tmp.replace("%s %d", "5");
    QListWidgetItem *item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(tmp);
    item1->setWhatsThis("300");
    ui->listWidget->addItem(item1);

    tmp = _("cloc_va_diff_hours_mins");
    tmp.replace("%s %d", "10");
    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(tmp);
    item1->setWhatsThis("600");
    ui->listWidget->addItem(item1);

    tmp = _("cloc_va_diff_hours_mins");
    tmp.replace("%s %d", "20");
    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(tmp);
    item1->setWhatsThis("1200");
    ui->listWidget->addItem(item1);


}

MDialog::~MDialog()
{
    delete ui;
}

void MDialog::on_listWidget_itemActivated(QListWidgetItem* item)
{
    selected = item->whatsThis().toInt();
    this->accept();
}
