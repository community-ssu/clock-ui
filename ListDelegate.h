#include <QMaemo5Style>
#include <QPainter>
#include <QFontMetrics>
#include <QStyledItemDelegate>
#include "Metrics.h"

class ListDelegate : public QStyledItemDelegate
{
	    Q_OBJECT

	public:
		 ListDelegate(QObject *parent) : QStyledItemDelegate(parent) { }
		 
void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
        using namespace Metrics::Item;

		QStyledItemDelegate::paint(painter, option, QModelIndex());

		QFont f = painter->font();
		QRect r = option.rect.adjusted(Margin, Margin+TextMargin, -Margin, -Margin-ValueMargin);

		painter->save();


        //GET TITLE, DESCRIPTION AND ICON
        QString title = index.data(Qt::DisplayRole).toString();
        QString description = index.data(Qt::UserRole).toString();

        //TITLE
        painter->drawText(r, Qt::AlignTop|Qt::AlignHCenter, title);

        // Draw weekday name
        f.setPointSize(13);
        painter->setFont(f);
		painter->setPen(QMaemo5Style::standardColor("SecondaryTextColor"));
        painter->drawText(r, Qt::AlignBottom|Qt::AlignHCenter, description);

		painter->restore();

}

QSize sizeHint ( const QStyleOptionViewItem&, const QModelIndex& ) const
{
   using namespace Metrics::Item;

   return QSize(1, Height);
}
};
