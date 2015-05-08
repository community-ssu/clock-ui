#ifndef QALARMVALUEBUTTON_H
#define QALARMVALUEBUTTON_H

#include <QMaemo5ValueButton>

class QAlarmValueButton : public QMaemo5ValueButton
{
    Q_OBJECT
public:
    explicit QAlarmValueButton(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *e);
    virtual QString valueText() const
    {
        return QMaemo5ValueButton::valueText();
    }

Q_SIGNALS:

public Q_SLOTS:

};

#endif // QALARMVALUEBUTTON_H
