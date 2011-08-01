#include "alarmbutton.h"
#include <QtMaemo5>
#include "osso-intl.h"
#include <QDebug>


AlarmButton::AlarmButton(QWidget * parent) : QMaemo5ValueButton(parent)
{
    this->setMinimumWidth(164);
    this->setMaximumWidth(164);
    this->setMinimumHeight(200);
    this->setMaximumHeight(200);
    connect ( this, SIGNAL( clicked(bool) ), this, SLOT( clicked() ) );
}

void AlarmButton::clicked()
{
    emit clicked(objectName());
}

void AlarmButton::paintEvent(QPaintEvent *paint)
{

    QSettings settings( "/etc/hildon/theme/colors.config", QSettings::IniFormat );
    QString color1 = settings.value("Colors/DefaultTextColor", "").toString();
    QString color2 = settings.value("Colors/ActiveTextColor", "").toString();

    QPainter p(this);
    QRect r = this->rect();

    p.drawPixmap(r, this->icon().pixmap(164,164) );

    QFont f = p.font();
    f.setPointSize(18);
    p.setFont(f);
    p.setPen(QPen(QColor(color1)));
    //QFontMetrics fm(f);
    //QString text = fm.elidedText(this->statusTip(), Qt::ElideRight, r.width()-40);
    r = this->rect();
    p.drawText(r.left(),r.top()+180,r.width(),r.height(),Qt::AlignVCenter|Qt::AlignLeft, this->text(), &r);

    //f.setPointSize(14);
    //p.setFont(f);

    r = this->rect();
    p.setPen(QPen(QColor(color1)));
    //p.drawText(r.left(),r.top()+180,r.width(),r.height(),Qt::AlignVCenter|Qt::AlignLeft,this->text(), &r);

    p.save();
    p.restore();

}
