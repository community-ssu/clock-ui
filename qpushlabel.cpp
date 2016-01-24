#include "qpushlabel.h"

#include <QMouseEvent>

QPushLabel::QPushLabel(QWidget *parent) :
    QLabel(parent),
    emitClicked(false),
    pressed(false)
{
    QSize s(360, 164);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    setMargin(16);
    setMinimumSize(s);
    setMaximumSize(s);
}

void QPushLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->buttons() == Qt::LeftButton)
    {
        setStyleSheet("border-image: url(/etc/hildon/theme/images/320IconHighlight.png);");
        pressed = emitClicked = true;
    }

    QLabel::mousePressEvent(ev);
}

void QPushLabel::mouseMoveEvent(QMouseEvent *ev)
{
    if (pressed)
    {
        if (emitClicked && !rect().contains(ev->pos()))
        {
            setStyleSheet("");
            emitClicked = false;
        }
        else if (!emitClicked && rect().contains(ev->pos()))
        {
            setStyleSheet("border-image: url(/etc/hildon/theme/images/320IconHighlight.png);");
            emitClicked = true;
        }
    }

    QLabel::mouseMoveEvent(ev);
}

void QPushLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    QLabel::mouseReleaseEvent(ev);

    pressed = false;
    if (emitClicked)
    {
        setStyleSheet("");
        emitClicked = false;
        Q_EMIT clicked();
    }
}
