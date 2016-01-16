#ifndef QALARMLABEL_H
#define QALARMLABEL_H

#include <QLabel>

class QAlarmLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QAlarmLabel(const QString &title, QWidget *parent = 0);
    void setText(const QStringList &lines);
protected:
    QString t;
    QString titleMarkup() const;
};

#endif // QALARMLABEL_H
