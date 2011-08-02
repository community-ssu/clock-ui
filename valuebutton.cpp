#include "valuebutton.h"
#include <QtMaemo5>
#include "osso-intl.h"
#include <QDebug>

QSettings settings( "/etc/hildon/theme/colors.config", QSettings::IniFormat );
QString color1 = settings.value("Colors/DefaultTextColor", "").toString();
QString color2 = settings.value("Colors/ActiveTextColor", "").toString();


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
    //if ( this->statusTip() == "" )
    //{
        QPainter p(this);
        QRect r = this->rect();

        QFont f = p.font();
        f.setBold(false);
        f.setPointSize(18);
        p.setFont(f);
        p.setPen(QPen(QColor(color1)));
        //QFontMetrics fm(f);
        //QString text = fm.elidedText(this->statusTip(), Qt::ElideRight, r.width()-40);
        p.drawText(20,0,this->width()-20,this->height(),Qt::AlignVCenter|Qt::AlignLeft,this->statusTip(), &r);

        //f.setPointSize(14);
        //p.setFont(f);
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

        p.setPen(QPen(QColor(color2)));
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
    //}
}
