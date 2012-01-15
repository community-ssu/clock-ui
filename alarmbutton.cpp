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

//void AlarmButton::paintEvent(QPaintEvent *paint)
void AlarmButton::paintEvent(QPaintEvent *)
{

    QPainter p(this);
    QRect r = this->rect();

    p.drawPixmap(r, this->icon().pixmap(164,164) );

    QFont f = p.font();
    f.setPointSize(18);
    p.setFont(f);
    p.setPen(QPen(QMaemo5Style::standardColor("DefaultTextColor")));

    r = this->rect();
    p.drawText(r.left(),r.top()+180,r.width(),r.height(),Qt::AlignVCenter|Qt::AlignLeft, this->text(), &r);

    p.save();
    p.restore();

} 
