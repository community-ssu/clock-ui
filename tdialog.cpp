#include <QDesktopWidget>
#include <QFileInfo>

#include "tdialog.h"
#include "ui_tdialog.h"
#include "osso-intl.h"
#include <QDialogButtonBox>
#include "gconfitem.h"
#include "home.h"

// default alarm signal settings window

TDialog::TDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TDialog)
{
    extern QString musicFile;
    this->setAttribute(Qt::WA_Maemo5AutoOrientation, true);
    ui->setupUi(this);

    selected = "nothing";

    this->setWindowTitle(_("cloc_ti_alarm_tone"));

    intl("calendar");
    ui->moreButton_landscape->button(QDialogButtonBox::Retry)->setText(_("cal_more_events"));
    ui->moreButton_portrait->button(QDialogButtonBox::Retry)->setText(_("cal_more_events"));
    intl("osso-connectivity-ui");
    ui->moreButton_landscape->button(QDialogButtonBox::Apply)->setText(_("conn_iaps_bd_done"));
    ui->moreButton_portrait->button(QDialogButtonBox::Apply)->setText(_("conn_iaps_bd_done"));
    intl("osso-clock");

    QRegExp fileExt( "\\.ogg$|\\.mp3$|\\.aac$" );

    QListWidgetItem *item1;

    GConfItem *item = new GConfItem("/apps/clock/alarm-custom");
    QString text = item->value().toString();

    if (musicFile.toLower().contains(fileExt) && ! musicFile.toLower().contains("/usr/share/sounds/ui-clock_alarm_"))
    	text = musicFile;
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    fileExt.setCaseSensitivity(cs);

    if ( text != "/"  )
    {
        item1 = new QListWidgetItem();
        item1->setTextAlignment(Qt::AlignCenter);
        item1->setWhatsThis(text);
        item1->setText( QFileInfo(text.remove(fileExt)).fileName() );
        ui->listWidget->addItem(item1);
    }

    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(_("cloc_fi_set_alarm_tone1"));
    item1->setWhatsThis("/usr/share/sounds/ui-clock_alarm_default.aac");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(_("cloc_fi_set_alarm_tone2"));
    item1->setWhatsThis("/usr/share/sounds/ui-clock_alarm_2.aac");
    ui->listWidget->addItem(item1);

    item1 = new QListWidgetItem();
    item1->setTextAlignment(Qt::AlignCenter);
    item1->setText(_("cloc_fi_set_alarm_tone3"));
    item1->setWhatsThis("/usr/share/sounds/ui-clock_alarm_3.aac");
    ui->listWidget->addItem(item1);

    connect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(orientationChanged()));
    this->orientationChanged();

    ui->listWidget->setCurrentRow(0);

    loop = g_main_loop_new (NULL, FALSE);

}

TDialog::~TDialog()
{
    delete ui;
}

void TDialog::reject()
{
    stopSound();
    this->hide();
}

void TDialog::stopSound()
{
    if ( g_main_loop_is_running(loop) )
    {
        g_main_loop_quit(loop);
        gst_element_set_state (player, GST_STATE_NULL);
    }
}

void TDialog::orientationChanged()
{

    if (QApplication::desktop()->screenGeometry().width() < QApplication::desktop()->screenGeometry().height()) {
        ui->moreButton_landscape->hide();
        ui->moreButton_portrait->show();
        this->setMinimumHeight((ui->listWidget->count()*70)+170);
        this->setMaximumHeight((ui->listWidget->count()*70)+170);
    } else {
        ui->moreButton_portrait->hide();
        ui->moreButton_landscape->show();
        this->setMinimumHeight((ui->listWidget->count()*70)+10);
        this->setMaximumHeight((ui->listWidget->count()*70)+10);
    }
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);

}

void TDialog::on_listWidget_itemActivated(QListWidgetItem* item)
{
    //selected = item->whatsThis();
    //this->accept();
    QString sended = "file://"+item->whatsThis();
    QByteArray ba = sended.toUtf8();
    const char *str1 = ba.data();

    stopSound();

    gst_init (NULL,NULL);
    player = gst_element_factory_make ("playbin2", "Multimedia Player");
    g_object_set (G_OBJECT (player), "uri", str1, NULL);
    g_assert (player != NULL);
    //gdouble v = 0.5;
    //g_object_set(player, "volume", v);
    gst_element_set_state (player, GST_STATE_PLAYING);
    g_main_loop_run(loop);


}

void TDialog::on_moreButton_landscape_clicked(QAbstractButton* button)
{
    stopSound();

    intl("calendar");
    QString tmp = _("cal_more_events");
    QRegExp fileExt( "\\.ogg$|\\.mp3$|\\.aac$" );
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
    fileExt.setCaseSensitivity(cs);

    intl("osso-clock");
    if ( button->text() == tmp )
    {
        Home *hw = new Home(this, "/home/user/MyDocs/.sounds");
        hw->exec();
        if ( hw->selected != "" )
        {
            if ( ui->listWidget->count() == 3 )
            {
                QListWidgetItem *item1 = new QListWidgetItem();
                item1->setTextAlignment(Qt::AlignCenter);
                item1->setWhatsThis(hw->selected);
                item1->setText( QFileInfo(hw->selected.remove(fileExt)).fileName() );
                ui->listWidget->insertItem(0, item1);
                orientationChanged();
                ui->listWidget->setCurrentRow(0);
            }
            else
            {
                ui->listWidget->item(0)->setWhatsThis(hw->selected);
                ui->listWidget->item(0)->setText( QFileInfo(hw->selected.remove(fileExt)).fileName() );
                ui->listWidget->setCurrentRow(0);
            }
        }
        delete hw;
    }
    else  // completed pressed
    {
	// return the full path/filename
        selected = ui->listWidget->currentItem()->whatsThis();
        this->accept();
    }

}

void TDialog::on_moreButton_portrait_clicked(QAbstractButton* button)
{
    on_moreButton_landscape_clicked(button);
}
