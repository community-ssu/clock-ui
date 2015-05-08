#include "qalarmvaluebutton.h"

#include <QMaemo5Style>
#include <QStylePainter>

QAlarmValueButton::QAlarmValueButton(QWidget *parent) :
    QMaemo5ValueButton(parent)
{
    setValueLayout(QMaemo5ValueButton::ValueBesideText);
}

void QAlarmValueButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QStylePainter p(this);

    /* Push button bevel */
    QStyleOptionButton button;
    initStyleOption(&button);
    p.drawControl(QStyle::CE_PushButtonBevel, button);

    QStyleOptionMaemo5ValueButton option;
    initStyleOption(&option);
    option.styles =
            QStyleOptionMaemo5ValueButton::ValueBesideText |
            QStyleOptionMaemo5ValueButton::PickButton;

    /* Contents */
    QRect r = style()->subElementRect(QStyle::SE_PushButtonContents, &button,
                                      this);
    /* text */
    r.moveLeft(20); /* why 20? nobody knows */
    option.rect = r;
    option.value = QString();
    p.drawControl(QStyle::CE_PushButtonLabel, option);

    /* valueText */
    r.moveLeft(170);
    option.rect = r;
    option.value = valueText();;
    option.text = QString();
    p.drawControl(QStyle::CE_PushButtonLabel, option);
}
