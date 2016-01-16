#ifndef QPUSHLABEL_H
#define QPUSHLABEL_H

#include <QLabel>

class QPushLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QPushLabel(QWidget *parent = 0);

Q_SIGNALS:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);
    bool emitClicked;
    bool pressed;
};

#endif // QPUSHLABEL_H
