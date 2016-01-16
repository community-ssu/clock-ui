#ifndef QALARMPUSHBUTTON_H
#define QALARMPUSHBUTTON_H

#include <QPushButton>
#include "qalarmlabel.h"

class QAlarmPushButton : public QWidget
{
    Q_OBJECT
public:
    explicit QAlarmPushButton(const QString &title, QWidget *parent = 0);
    void setIcon(const QString &icon);
    void setText(const QStringList &lines)
    {
        label->setText(lines);
        updateGeometry();
    }
    typedef enum {
        Right,
        Bottom
    } Position;
    void setLabelPosition(QAlarmPushButton::Position pos);

private:
    QString iconName;
    QPushButton *button;
    QAlarmLabel *label;

private Q_SLOTS:
    void pressed();
    void released();
    void buttonClicked();

Q_SIGNALS:
    void clicked();
};

#endif // QALARMPUSHBUTTON_H
