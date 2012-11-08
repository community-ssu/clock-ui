#include "home.h"
#include "ui_home.h"
#include "qsettings.h"
#include "qmaemo5rotator.h"
#include <qdir.h>
#include <qfileinfo.h>
#include "osso-intl.h"


QString hPath;
QIcon ich;
QSettings hsettings("worldclock", "filemanager");
QSettings hfoldericons("worldclock", "filemanager_folders");
QSettings hfileicons("worldclock", "filemanager_icons");


Home::Home(QWidget *parent, QString path) :
    QDialog(parent),
    ui(new Ui::Home)
{
    ui->setupUi(this);

    selected = "";

    this->setWindowTitle(_("cloc_ti_open_sound_clip"));
    ui->folderButton_up->setIcon( QIcon::fromTheme("filemanager_folder_up") );

    QFileInfo dr(path);

    ui->folderbutton->setIcon( QIcon::fromTheme("general_folder") );
    ui->folderbutton->setText( dr.fileName() );
    if ( path == "/") ui->folderbutton->setText( "/" );
    ui->folderbutton->setValueText( path );
    if ( path == "/") ui->folderbutton->setValueText( "" );

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
    ui->folderbutton->setText( QFileInfo(directorio).fileName() );
    if ( directorio == "/") ui->folderbutton->setText( "/" );
    ui->folderbutton->setValueText( directorio );
    if ( directorio == "/") ui->folderbutton->setValueText( "" );

    ui->listWidget->clear();

    dir.setFilter( QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot );
    dir.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase );

    QRegExp fileExt( "\\.ogg$|\\.mp3$|\\.aac$" );
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
          else if (fileInfo.fileName().toLower().contains(fileExt))
        {
            QListWidgetItem *item1 = new QListWidgetItem( ui->listWidget );
            item1->setText(fileInfo.fileName());
            item1->setIcon(QIcon::fromTheme("general_audio_file"));
            ui->listWidget->insertItem( i, item1 );
        }

    }

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));

    if ( ui->listWidget->count() > 0 ) {
        ui->listWidget->scrollToItem(ui->listWidget->item(0));
    }

}

void Home::orientationChanged()
{
    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height())
    {
        this->resize(398, 692);
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

void Home::on_folderButton_up_clicked()
{
    if ( hPath == "/" ) return;
    QString nPath = hPath;
    int i = nPath.lastIndexOf( "/" );
    nPath.remove ( i, nPath.length() - i );
    if ( nPath == "" ) nPath = "/";
    CargarBrowser( nPath );

}

