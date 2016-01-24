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

    /* Contents */
    QStyleOptionMaemo5ValueButton option;
    initStyleOption(&option);

    option.styles = valueLayout() == ValueBesideText ?
                QStyleOptionMaemo5ValueButton::ValueBesideText :
                QStyleOptionMaemo5ValueButton::ValueUnderText;
    option.styles |= QStyleOptionMaemo5ValueButton::PickButton;

    QRect r = style()->subElementRect(QStyle::SE_PushButtonContents, &button,
                                      this);
    r.setLeft(20); /* why 20? nobody knows */

    option.rect = r;

    if (valueLayout() == ValueBesideText)
    {
        /* text */
        option.value = QString();
        p.drawControl(QStyle::CE_PushButtonLabel, option);

        /* valueText */
        r.setLeft(170);
        option.rect = r;
        option.value = valueText();
        option.text = QString();
    }
    else
        option.value = valueText();

    p.drawControl(QStyle::CE_PushButtonLabel, option);
}
