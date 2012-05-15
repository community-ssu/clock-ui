#ifndef CHECKDELEGATE_H
#define CHECKDELEGATE_H

#include <QStyledItemDelegate>

class CheckDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    CheckDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index ) const;
};

#endif
