#ifndef FILEDELEGATE_H
#define FILEDELEGATE_H

#include <QItemDelegate>

class FileDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    FileDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const;

};

#endif
