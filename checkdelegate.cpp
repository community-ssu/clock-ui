#include <QItemDelegate>
#include <QStandardItemModel>
#include <QPainter>
#include <QDirModel>
#include <QDebug>
#include <QListWidgetItem>
#include <QDateTime>
#include "checkdelegate.h"
#include "osso-intl.h"

CheckDelegate::CheckDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void CheckDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
  //QStyledItemDelegate::paint(painter,option,index);
  QString name = index.data(Qt::DisplayRole).toString();
  QString sel = index.data(Qt::UserRole+1).toString();

  painter->save();
  QRect r = option.rect;

  if(option.state & QStyle::State_Selected){
      QPixmap is1 = QPixmap("/etc/hildon/theme/images/TouchListBackgroundPressed.png");
      painter->drawTiledPixmap(r,is1);
  } else {
      QPixmap is1 = QPixmap("/etc/hildon/theme/images/TouchListBackgroundNormal.png" );
      painter->drawTiledPixmap(r,is1);
  }

  if( sel == "selected")
  {
      r = option.rect;
      QPixmap is1 = QPixmap("/usr/share/icons/hicolor/48x48/hildon/widgets_tickmark_list.png");
      painter->drawPixmap(r.width()-60,r.top()+11,48,48,is1);
  }

  name.replace("1", QDate::longDayName(1) );
  name.replace("2", QDate::longDayName(2) );
  name.replace("3", QDate::longDayName(3) );
  name.replace("4", QDate::longDayName(4) );
  name.replace("5", QDate::longDayName(5) );
  name.replace("6", QDate::longDayName(6) );
  name.replace("7", QDate::longDayName(7) );
  name.replace("0", _("cloc_va_never") );
  name.replace("8", _("cloc_va_everyday") );
  //name[0] = name[0].toUpper();

  painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignCenter, name, &r);
  painter->restore();

}


QSize CheckDelegate::sizeHint(const QStyleOptionViewItem &,
                              const QModelIndex &) const
{
      return QSize(5, 70);
}

