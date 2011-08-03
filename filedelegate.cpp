#include <QItemDelegate>
#include <QStandardItemModel>
#include <QPainter>
#include <QDirModel>
#include <QSettings>
#include <QDebug>
#include "osso-intl.h"
#include "filedelegate.h"
#include <QDateTime>

FileDelegate::FileDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{


}

void FileDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{

  painter->save();
  QRect r = option.rect;

  if(option.state & QStyle::State_Selected){
      QPixmap is1 = QPixmap("/etc/hildon/theme/images/TouchListBackgroundPressed.png");
      painter->drawTiledPixmap(r,is1);
  } else {
      QPixmap is1 = QPixmap("/etc/hildon/theme/images/TouchListBackgroundNormal.png" );
      painter->drawTiledPixmap(r,is1);
  }


  QSettings settings( "/etc/hildon/theme/index.theme", QSettings::IniFormat );
  QString currtheme = settings.value("X-Hildon-Metatheme/IconTheme","hicolor").toString();

  if ( index.data(Qt::DisplayRole) == "active" )
      painter->drawPixmap(r.left(), r.top()+12, QPixmap("/usr/share/icons/"+currtheme+"/48x48/hildon/clock_alarm_on.png"));
  else if ( index.data(Qt::DisplayRole) == "inactive" )
      painter->drawPixmap(r.left(), r.top()+12, QPixmap("/usr/share/icons/"+currtheme+"/48x48/hildon/clock_alarm_off.png"));

  QColor base = QColor(Qt::white);
  painter->setPen(base);

  QFont f = painter->font();
  QString name = index.data(Qt::DisplayRole).toString();
  QString help = index.data(Qt::WhatsThisRole).toString();

  if ( help == "time" )
  {
      QString name2 = "";
      if ( name.contains( "a.m.") )
      {
          name2 = "a.m.";
          name.remove("a.m.");
      }
      if ( name.contains( "p.m.") )
      {
          name2 = "p.m.";
          name.remove("p.m.");
      }
      if ( name.contains( "am") )
      {
          name2 = "am";
          name.remove("am");
      }
      if ( name.contains( "pm") )
      {
          name2 = "pm";
          name.remove("pm");
      }

      f.setPointSize( f.pointSize() + 8 );
      painter->setFont(f);
      r = option.rect;
      painter->drawText(r.left()+6, r.top(), 92, r.height(), Qt::AlignVCenter|Qt::AlignRight, name, &r);
      f.setPointSize( f.pointSize() - 12 );
      painter->setFont(f);
      r = option.rect;
      painter->drawText(r.left()+96, r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, name2, &r);
  }

  if ( help == "name" )
  {
      painter->setFont(f);
      r = option.rect;
      painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, name, &r);
  }

  if ( help == "days" )
  {
      base = QColor(Qt::gray);
      painter->setPen(base);
      f.setPointSize( f.pointSize()-4 );
      painter->setFont(f);
      r = option.rect;

      int i = name.count(",");
      if ( i > 2 )
      {
          QString l1;
          i = name.indexOf(",");
          l1 = name.left(i+1);
          name.remove(0, i+1);
          i = name.indexOf(",");
          l1 = l1 + name.left(i+1);
          name.remove(0, i+1);
          i = name.indexOf(",");
          l1 = l1 + name.left(i);
          name.remove(0,i+2);

          name = l1 + "\n" + name;
          name.replace("1", QDate::shortDayName(1) );
          name.replace("2", QDate::shortDayName(2) );
          name.replace("3", QDate::shortDayName(3) );
          name.replace("4", QDate::shortDayName(4) );
          name.replace("5", QDate::shortDayName(5) );
          name.replace("6", QDate::shortDayName(6) );
          name.replace("7", QDate::shortDayName(7) );
      }
      else
      {
          name.replace("1", QDate::longDayName(1) );
          name.replace("2", QDate::longDayName(2) );
          name.replace("3", QDate::longDayName(3) );
          name.replace("4", QDate::longDayName(4) );
          name.replace("5", QDate::longDayName(5) );
          name.replace("6", QDate::longDayName(6) );
          name.replace("7", QDate::longDayName(7) );
          name.replace("0", _("cloc_va_never") );
          name.replace("8", _("cloc_va_everyday") );
      }


      painter->drawText(r.left(), r.top(), r.width()-12, r.height(), Qt::AlignVCenter|Qt::AlignRight|Qt::TextWordWrap, name, &r);

  }

  if ( help == "world-name" )
  {
      QString tmp = name;
      int j = tmp.indexOf("startdesc");
      tmp.remove(0, j+9);
      name.remove(tmp);
      name.remove("startdesc");
      painter->setFont(f);
      r = option.rect;
      if ( r.width() < 400 )
          name = name + "\n";
      painter->drawText(r.left(), r.top(), r.width(), r.height(), Qt::AlignVCenter|Qt::AlignLeft, name, &r);
      QFontMetrics fm(f);
      int len = fm.width(name);
      base = QColor(Qt::gray);
      painter->setPen(base);
      painter->setFont(f);
      //tmp = fm.elidedText(tmp, Qt::ElideRight, r.width()-len);
      r = option.rect;
      int h1 = 0;
      if ( r.width() < 400 )
      {
        len = 0;
        h1 = 30;
        f.setPointSize(14);
        painter->setFont(f);
      }
      painter->drawText(r.left()+len, r.top()+h1, r.width()-len, r.height()-h1, Qt::AlignVCenter|Qt::AlignLeft, tmp, &r);

  }

  if ( help == "world-date" )
  {
      base = QColor(Qt::gray);
      painter->setPen(base);
      f.setPointSize(14);
      painter->setFont(f);
      QString tmp = name;
      int j = tmp.indexOf("  startdesc");
      tmp.remove(0, j+11);
      name.remove(tmp);
      name.remove("  startdesc");
      painter->setFont(f);
      r = option.rect;
      if ( r.width() < 200 )
          name = name + "\n";
      else
          name = name + "   (" + tmp + ")";
      painter->drawText(r.left(), r.top(), r.width()-6, r.height(), Qt::AlignVCenter|Qt::AlignRight, name, &r);
      painter->setFont(f);
      r = option.rect;
      if ( r.width() < 200 )
          painter->drawText(r.left(), r.top()+30, r.width()-6, r.height()-30, Qt::AlignVCenter|Qt::AlignRight, "("+tmp+")", &r);
  }



  painter->restore();

}


QSize FileDelegate::sizeHint(const QStyleOptionViewItem &,
                              const QModelIndex &) const
{
      return QSize( 0, 70);
}

