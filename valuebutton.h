#ifndef VALUEBUTTON_H
#define VALUEBUTTON_H

//#include <QPushButton>
#include <QtMaemo5>

class QString;
class QWidget;

class ValueButton : public QMaemo5ValueButton {

Q_OBJECT

public:
    ValueButton(QWidget * parent = NULL);

private slots:
    void clicked();

signals:
    void clicked(const QString &button);

protected:
    void paintEvent(QPaintEvent *);


};

#endif // VALUEBUTTON_H
