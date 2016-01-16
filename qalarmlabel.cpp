#include "qalarmlabel.h"

#include <QMaemo5Style>

static const QString styleNoMargin =
        "margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;"
        "-qt-block-indent:0; text-indent:0px;";

QAlarmLabel::QAlarmLabel(const QString &title, QWidget *parent) :
    QLabel(parent),
    t(title)
{
    setText(QStringList());
}

void QAlarmLabel::setText(const QStringList &lines)
{
    QString markup = titleMarkup();

    for (int i = 0; i < lines.count(); i ++)
    {
        markup +=
                "<p align=center "
                "style=\"" + styleNoMargin + "font-size:small; color:" +
                QMaemo5Style::standardColor("SecondaryTextColor").name() +
                ";\">" + lines.at(i) + "</p>";
    }
    QLabel::setText(markup);
}

QString QAlarmLabel::titleMarkup() const
{
    return "<p align=center style=\"" + styleNoMargin + "font-size:small;\">" +
            t + "</p>";
}

