#include "qmaemo5rotator.h"
#include "dialog.h"
#include "ui_dialog.h"
#include "osso-intl.h"
#include "checkdelegate.h"

Dialog::Dialog(QWidget *parent, QString name, QString list) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->setWindowTitle(name);

    result.clear();

    CheckDelegate *pluginDelegate = new CheckDelegate(this);
    ui->listWidget->setItemDelegate(pluginDelegate);

    intl("osso-connectivity-ui");
    ui->buttonBox->button(QDialogButtonBox::Save)->setText(_("conn_iaps_bd_done"));
    ui->buttonBox_2->button(QDialogButtonBox::Save)->setText(_("conn_iaps_bd_done"));
    intl("osso-clock");


    QListWidgetItem *item1 = new QListWidgetItem("0");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis(_("cloc_va_never"));
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("1");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("1");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("2");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("2");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("3");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("3");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("4");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("4");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("5");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("5");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("6");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("6");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("7");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis("7");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem("8");
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setWhatsThis(_("cloc_va_everyday"));
    ui->listWidget->addItem(item1);


    if ( list.contains("0") ) ui->listWidget->item(0)->setSelected(true);
    if ( list.contains("1") ) ui->listWidget->item(1)->setSelected(true);
    if ( list.contains("2") ) ui->listWidget->item(2)->setSelected(true);
    if ( list.contains("3") ) ui->listWidget->item(3)->setSelected(true);
    if ( list.contains("4") ) ui->listWidget->item(4)->setSelected(true);
    if ( list.contains("5") ) ui->listWidget->item(5)->setSelected(true);
    if ( list.contains("6") ) ui->listWidget->item(6)->setSelected(true);
    if ( list.contains("7") ) ui->listWidget->item(7)->setSelected(true);
    if ( list.contains("8") )
        for (int i=1; i<ui->listWidget->count(); ++i)
            ui->listWidget->item(i)->setSelected(true);

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::orientationChanged()
{
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        ui->buttonBox->hide();
        ui->buttonBox_2->show();
        this->setMinimumHeight(680);
        this->setMaximumHeight(680);
    } else {
        ui->buttonBox_2->hide();
        ui->buttonBox->show();
        this->setMinimumHeight(350);
        this->setMaximumHeight(350);
    }
}

void Dialog::on_listWidget_itemActivated(QListWidgetItem* )
{

    if ( ui->listWidget->currentRow() != 0 && ui->listWidget->selectedItems().count() > 0 )
        ui->listWidget->item(0)->setSelected(false);
    if ( ui->listWidget->selectedItems().count() == 0 )
        ui->listWidget->item(0)->setSelected(true);

    if ( ui->listWidget->currentRow() == 8 )
    {
            ui->listWidget->item(0)->setSelected(false);
            ui->listWidget->item(1)->setSelected(true);
            ui->listWidget->item(2)->setSelected(true);
            ui->listWidget->item(3)->setSelected(true);
            ui->listWidget->item(4)->setSelected(true);
            ui->listWidget->item(5)->setSelected(true);
            ui->listWidget->item(6)->setSelected(true);
            ui->listWidget->item(7)->setSelected(true);
    }

    if ( ui->listWidget->item(1)->isSelected() && ui->listWidget->item(2)->isSelected() &&
         ui->listWidget->item(3)->isSelected() && ui->listWidget->item(4)->isSelected() &&
         ui->listWidget->item(5)->isSelected() && ui->listWidget->item(6)->isSelected() &&
         ui->listWidget->item(7)->isSelected() )
        ui->listWidget->item(8)->setSelected(true);
    else
        ui->listWidget->item(8)->setSelected(false);

    if ( ui->listWidget->item(0)->isSelected() )
    {
        ui->listWidget->item(1)->setSelected(false);
        ui->listWidget->item(2)->setSelected(false);
        ui->listWidget->item(3)->setSelected(false);
        ui->listWidget->item(4)->setSelected(false);
        ui->listWidget->item(5)->setSelected(false);
        ui->listWidget->item(6)->setSelected(false);
        ui->listWidget->item(7)->setSelected(false);
        ui->listWidget->item(8)->setSelected(false);
    }
    else if ( ui->listWidget->item(8)->isSelected() )
    {
        ui->listWidget->item(0)->setSelected(false);
        ui->listWidget->item(1)->setSelected(true);
        ui->listWidget->item(2)->setSelected(true);
        ui->listWidget->item(3)->setSelected(true);
        ui->listWidget->item(4)->setSelected(true);
        ui->listWidget->item(5)->setSelected(true);
        ui->listWidget->item(6)->setSelected(true);
        ui->listWidget->item(7)->setSelected(true);
    }

}


void Dialog::on_listWidget_itemSelectionChanged()
{
    for (int i=0; i < ui->listWidget->count(); ++i)
    {
        if ( ui->listWidget->item(i)->isSelected() )
            ui->listWidget->item(i)->setData(Qt::UserRole+1, "selected");
        else
            ui->listWidget->item(i)->setData(Qt::UserRole+1, "");
    }

}

void Dialog::on_buttonBox_accepted()
{
    QStringList lista;
    for ( int i=0; i < ui->listWidget->count(); ++i )
    {
        if ( ui->listWidget->item(i)->isSelected() )
            lista.append(ui->listWidget->item(i)->text() );
    }
    if ( lista.contains("8") )
    {
        lista.clear();
        lista.append("8");
    }

    result = lista;
    this->accept();
}

void Dialog::on_buttonBox_2_accepted()
{
    on_buttonBox_accepted();
}
