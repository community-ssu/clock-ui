#ifndef ALARMBUTTON_H
#define ALARMBUTTON_H

//#include <QPushButton>
#include <QtMaemo5>

class QString;
class QWidget;

class AlarmButton : public QMaemo5ValueButton {

Q_OBJECT

public:
    AlarmButton(QWidget * parent = NULL);

private slots:
    void clicked();

signals:
    void clicked(const QString &button);

protected:
    void paintEvent(QPaintEvent *);


};

#endif // ALARMBUTTON_H
