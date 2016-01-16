#ifndef QALARMSOUNDVALUEBUTTON_H
#define QALARMSOUNDVALUEBUTTON_H

#include "qalarmvaluebutton.h"

class QAlarmSoundValueButton : public QAlarmValueButton
{
    Q_OBJECT
public:
    explicit QAlarmSoundValueButton(QWidget *parent = 0);
    bool isStandardSound()
    {
        return standart;
    }

public Q_SLOTS:
    void setSoundFile(const QString &file);
    QString getSoundFile() const;
protected:
    bool standart;
};

#endif // QALARMSOUNDVALUEBUTTON_H
