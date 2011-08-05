/**************************************************************************
    This file is part of Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include "maintdelegate.h"
#include <QSettings>

void MainDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter,option,index);
    QString title = index.data(Qt::UserRole).toString();
    QString subtitle = index.data(Qt::UserRole+2).toString();
    QString subtitle2 = index.data(Qt::UserRole+3).toString();
    QString icon = index.data(Qt::UserRole+1).toString();

    painter->save();

    QSettings settings2( "/etc/hildon/theme/colors.config", QSettings::IniFormat );
    QString color = settings2.value("Colors/DefaultTextColor", "").toString();
    QString color2 = settings2.value("Colors/SecondaryTextColor", "").toString();

    QRect r = option.rect;
    QFont f = painter->font();
    QColor gray;
    gray = QColor(156, 154, 156);

    r = option.rect;
    painter->drawPixmap(r.left(), r.top(), 164, 164, icon);

    r = option.rect;
    f.setPointSize(18);
    painter->setFont(f);
    painter->setPen(QPen(QColor(color)));

    int space = 4;
    if ( subtitle != "" )
        space = 36;
    if ( subtitle2 != "" )
        space = 74;

    painter->drawText(170, r.top(), r.width()-170, r.height()-space, Qt::AlignVCenter|Qt::AlignLeft, title, &r);

    r = option.rect;
    f.setPointSize(13);
    painter->setFont(f);
    painter->setPen(QPen(QColor(color2)));


    space = 92;
    if ( subtitle2 != "" )
        space = 72;

    r = option.rect;
    painter->drawText(170, r.top()+space, r.width()-170, 20, Qt::AlignVCenter|Qt::AlignLeft, subtitle, &r);

    r = option.rect;
    painter->drawText(170, r.top()+106, r.width()-170, 20, Qt::AlignVCenter|Qt::AlignLeft, subtitle2, &r);

    painter->restore();
}

QSize MainDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
        return QSize(400, 164);
}
