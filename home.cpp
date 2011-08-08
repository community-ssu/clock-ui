#include "home.h"
#include "ui_home.h"
#include "qsettings.h"
#include "qmaemo5rotator.h"
#include <qdir.h>
#include <qfileinfo.h>
#include "osso-intl.h"


QString hPath;
QIcon ich;
QSettings hsettings("cepiperez", "filemanager");
QSettings hfoldericons("cepiperez", "filemanager_folders");
QSettings hfileicons("cepiperez", "filemanager_icons");


Home::Home(QWidget *parent, QString path) :
    QDialog(parent),
    ui(new Ui::Home)
{
    ui->setupUi(this);

    selected = "";

    this->setWindowTitle(_("cloc_ti_open_sound_clip"));
    ui->pushButton->setIcon( QIcon::fromTheme("filemanager_folder_up") );

    QFileInfo dr(path);

    ui->button->setIcon( QIcon::fromTheme("general_folder") );
    ui->button->setText( dr.fileName() );
    if ( path == "/") ui->button->setText( "/" );
    ui->button->setValueText( path );
    if ( path == "/") ui->button->setValueText( "" );

    CargarBrowser( path );

    setAttribute(Qt::WA_Maemo5AutoOrientation, true);

}

Home::~Home()
{
    delete ui;
}

void Home::CargarBrowser(QString directorio)
{

    hPath = directorio;
    QDir dir ( directorio, "*" );
    dir.setFilter ( QDir::Dirs | QDir::Hidden );
    if ( !dir.isReadable() )
          return;
    ui->button->setText( QFileInfo(directorio).fileName() );
    if ( directorio == "/") ui->button->setText( "/" );
    ui->button->setValueText( directorio );
    if ( directorio == "/") ui->button->setValueText( "" );

    ui->listWidget->clear();

    dir.setFilter( QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot );
    dir.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase );

    //dir.setSorting( QDir::DirsFirst );
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
      QFileInfo fileInfo = list.at(i);

        if ( fileInfo.isDir() )
        {
            QListWidgetItem *item1 = new QListWidgetItem( ui->listWidget );
            item1->setText(fileInfo.fileName());
            item1->setIcon(QIcon::fromTheme("general_folder"));
            ui->listWidget->insertItem( i, item1 );
        }
        else if ( (fileInfo.completeSuffix().toLower()=="mp3") ||
                  (fileInfo.completeSuffix().toLower()=="ogg") ||
                  (fileInfo.completeSuffix().toLower()=="aac") )
        {
            QListWidgetItem *item1 = new QListWidgetItem( ui->listWidget );
            item1->setText(fileInfo.fileName());
            item1->setIcon(QIcon::fromTheme("general_audio_file"));
            ui->listWidget->insertItem( i, item1 );
        }

    }


    if ( ui->listWidget->count() > 0 ) {
        ui->listWidget->scrollToItem(ui->listWidget->item(0));
    }

}

void Home::on_listWidget_itemClicked(QListWidgetItem* item)
{

    QString temp = hPath;
    if ( temp != "/" ) temp.append("/");
    temp.append( item->text() );

    if ( QFileInfo(temp).isFile() )
    {
        selected = temp;
        this->accept();
    }
    else if ( QFileInfo(temp).isDir() )
    {
        CargarBrowser( temp );
    }



}

void Home::on_pushButton_clicked()
{
    if ( hPath == "/" ) return;
    QString nPath = hPath;
    int i = nPath.lastIndexOf( "/" );
    nPath.remove ( i, nPath.length() - i );
    if ( nPath == "" ) nPath = "/";
    CargarBrowser( nPath );

}

