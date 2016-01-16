#include <QListView>
#include <osso-intl.h>

#include "qalarmsnoozevaluebutton.h"

#define MAX_SNOOZE_ITEMS 4

QWidget *QAlarmSnoozeListPickSelector::widget(QWidget *parent)
{
    QWidget *widget;
    QListView *_view = new QListView();

    _view->setModel(model());

    if (_view->sizeHintForRow(0) > 0)
        _view->setMinimumHeight(_view->sizeHintForRow(0) *
                               (MAX_SNOOZE_ITEMS < 5 ? MAX_SNOOZE_ITEMS : 5));

    setView(_view);
    widget = QMaemo5ListPickSelector::widget(parent);

    return widget;
}

QAlarmSnoozeValueButton::QAlarmSnoozeValueButton(QWidget *parent) :
    QAlarmValueButton(parent),
    selector(new QAlarmSnoozeListPickSelector(this))
{
    setText(_("dati_fi_snooze_time"));

    for (int i = 1; i <= MAX_SNOOZE_ITEMS; i ++)
    {
        QStandardItem *item;
        QString rowText =_("cloc_va_diff_hours_mins");

        rowText.replace("%s %d", QString::number(i * 5));
        item = new QStandardItem(rowText);
        item->setTextAlignment(Qt::AlignCenter);
        item->setEditable(false);

        model.appendRow(item);
    }

    selector->setModel(&model);

    setPickSelector(selector);
    connect(selector, SIGNAL(selected(QString)),
            this, SLOT(snoozeSelected(QString)));
}

void QAlarmSnoozeValueButton::setSnooze(int minutes)
{
    int i = (minutes / 5) - 1;

    if (i >= MAX_SNOOZE_ITEMS)
        i = MAX_SNOOZE_ITEMS - 1;

    selector->setCurrentIndex(i);
}

int QAlarmSnoozeValueButton::getSnooze()
{
    return (selector->currentIndex() + 1) * 5;
}

void QAlarmSnoozeValueButton::snoozeSelected(QString)
{
    emit selected(getSnooze());
}
