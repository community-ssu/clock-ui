#include "valuebutton.h"
#include <QtMaemo5>
#include "osso-intl.h"
#include <QDebug>


ValueButton::ValueButton(QWidget * parent) : QMaemo5ValueButton(parent)
{
    setMinimumWidth(90);
    connect ( this, SIGNAL( clicked(bool) ), this, SLOT( clicked() ) );
}

void ValueButton::clicked()
{
    emit clicked(objectName());
}

void ValueButton::paintEvent(QPaintEvent *paint)
{
    QPushButton::paintEvent(paint);
    QPainter p(this);
    QRect r = this->rect();

    QFont f = p.font();
    f.setBold(false);
    f.setPointSize(18);
    p.setFont(f);
    p.setPen(QPen(QMaemo5Style::standardColor("DefaultTextColor")));

    p.drawText(20,0,this->width()-20,this->height(),Qt::AlignVCenter|Qt::AlignLeft,this->statusTip(), &r);

    QString name = this->valueText();

    if ( this->whatsThis() == "date" )
    {
        name.replace("1", QDate::shortDayName(1) );
        name.replace("2", QDate::shortDayName(2) );
        name.replace("3", QDate::shortDayName(3) );
        name.replace("4", QDate::shortDayName(4) );
        name.replace("5", QDate::shortDayName(5) );
        name.replace("6", QDate::shortDayName(6) );
        name.replace("7", QDate::shortDayName(7) );
        name.replace("0", _("cloc_va_never") );
        name.replace("8", _("cloc_va_everyday") );
    }

    p.setPen(QPen(QMaemo5Style::standardColor("ActiveTextColor")));
    if ( this->whatsThis() != "alarm" )
        p.drawText(140,0,this->width()-140,this->height(),Qt::AlignVCenter|Qt::AlignLeft,name, &r);
    else
    {
        QFontMetrics fm(f);
        if ( QFileInfo(name).isFile() )
            name = QFileInfo(name).baseName();
        name = fm.elidedText(name, Qt::ElideRight, this->width()-250);
        p.drawText(240,0,this->width()-240,this->height(),Qt::AlignVCenter|Qt::AlignLeft,name, &r);
    }

    p.save();
    p.restore();

}
