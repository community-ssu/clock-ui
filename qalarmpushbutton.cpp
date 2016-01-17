#include "qalarmpushbutton.h"
#include <QDebug>
#include <QLayout>

QAlarmPushButton::QAlarmPushButton(const QString &title, QWidget *parent) :
    QWidget(parent),
    button(new QPushButton(this)),
    label(new QAlarmLabel(title))
{
}

void QAlarmPushButton::setIcon(const QString &icon)
{
    iconName = icon;

    QSize s(164, 164);
    setMinimumSize(s);

    button->setMinimumSize(s);
    button->setMaximumSize(s);
    button->setIconSize(s);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFlat(true);

    button->setStyleSheet(
        "QPushButton:pressed {"
            "border-image: url(/etc/hildon/theme/images/164IconHighlight.png);"
        "}");
    button->setIcon(QIcon::fromTheme(iconName));

    connect(button, SIGNAL(pressed()), this, SLOT(pressed()));
    connect(button, SIGNAL(released()), this, SLOT(released()));
    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));

    setLayout(new QVBoxLayout);
    setLabelPosition(QAlarmPushButton::Bottom);
}

void QAlarmPushButton::setLabelPosition(QAlarmPushButton::Position pos)
{
    setUpdatesEnabled(false);
    layout()->removeWidget(button);
    layout()->removeWidget(label);
    delete layout();

    if (pos == QAlarmPushButton::Right)
    {
        setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
        setLayout(new QHBoxLayout);

        layout()->setAlignment(Qt::AlignLeft | Qt::AlignTop);
        layout()->setContentsMargins(32, 0, 32, 0);
        layout()->setSpacing(32);

        label->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
    }
    else
    {
        setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
        setLayout(new QVBoxLayout);

        layout()->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        layout()->setContentsMargins(0, 0, 0, 0);
        layout()->setSpacing(0);

        label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    }

    layout()->addWidget(button);
    layout()->addWidget(label);

    setUpdatesEnabled(true);
}

void QAlarmPushButton::pressed()
{
    button->setIcon(QIcon::fromTheme(iconName));
}

void QAlarmPushButton::released()
{
    button->setIcon(QIcon::fromTheme(iconName + "_pressed"));
}

void QAlarmPushButton::buttonClicked()
{
    emit clicked();
}
