#include "qalarmdialog.h"
#include "qnewalarmdialog.h"
#include "osso-intl.h"

#include <QMaemo5Style>
#include <QDesktopWidget>
#include <QModelIndex>
#include <QVBoxLayout>
#include <QDBusConnection>
#include <QTimer>
#include <QMouseEvent>
#include <QCoreApplication>

#include "utils.h"

enum {
    AlarmEnabledRole = Qt::UserRole + 1,
    AlarmCookieRole,
    AlarmWdayRole,
    AlarmDateTimeRole
};

QAlarmDialog::QAlarmDialog(QWidget *parent) :
    QDialog(parent),
    label(new QLabel),
    button(new QPushButton()),
    view(new QAlarmTreeView()),
    model(new QStandardItemModel(0, 4, view))
{
    setWindowTitle(_("cloc_ti_alarms"));
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setWindowFlags(Qt::Window);

    /* model/view */
    model->setSortRole(AlarmEnabledRole);

    view->setModel(model);
    view->setColumnWidth(0, 64);
    view->setColumnWidth(1, 96);
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setIconSize(QSize(48, 48));
    view->setIndentation(0);
    view->setUniformRowHeights(true);
    view->setHeaderHidden(true);

    /* button */
    button->setIconSize(QSize(48, 48));
    button->setIcon(QIcon::fromTheme("general_add"));
    button->setText(_("clock_ti_new_alarm"));

    /* label */
    QPalette pal(palette());
    pal.setColor(QPalette::WindowText,
                 QMaemo5Style::standardColor("SecondaryTextColor"));
    label->setPalette(pal);
    label->setFont(QMaemo5Style::standardFont("XX-LargeSystemFont"));
    label->setAlignment(Qt::AlignCenter);
    label->setText(_("cloc_ti_start_no"));

    /* put them all together */
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(button);
    layout->addWidget(label);
    layout->addWidget(view);

    /* if someone has connected to our changed signals to have the real data */
    QTimer::singleShot(0, this, SLOT(addAlarms()));

    /* refresh the current alarmlist as soon as an alarm event occurs */
    QDBusConnection::systemBus().connect(QString(), "/com/nokia/alarmd",
                                         "com.nokia.alarmd", "queue_status_ind",
                                         this, SLOT(addAlarms()));
    connect(view, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(viewClicked(const QModelIndex &)));

    connect(button, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

QStandardItem *QAlarmDialog::alarmCheckboxItem(cookie_t cookie,
                                             const alarm_event_t *ae)
{
    QStandardItem *item = new QStandardItem();

    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setEditable(false);

    /* enable/disable check box */
    if (!(ae->flags & ALARM_EVENT_DISABLED))
    {
        item->setIcon(QIcon::fromTheme("clock_alarm_on").pixmap(48, 48));
        item->setData(1, AlarmEnabledRole);
    }
    else
    {
        item->setIcon(QIcon::fromTheme("clock_alarm_off").pixmap(48, 48));
        item->setData(0, AlarmEnabledRole);
    }

    item->setData((qlonglong)cookie, AlarmCookieRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmTimeItem(const alarm_event_t *ae)
{
    QStandardItem *item = new QStandardItem();
    time_t tick = ae->snooze_total;

    if (!tick)
        tick = ae->trigger;

    item->setText(formatDateTime(tick, Time));
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setFont(QMaemo5Style::standardFont("LargeSystemFont"));

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    /* needed so sorting to work */
    item->setData(item->text());
    item->setData((int)tick, AlarmDateTimeRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmTitleItem(const alarm_event_t *ae)
{
    const char *s = alarm_event_get_message(ae);

    if(!s)
        s = alarm_event_get_title(ae);

    QStandardItem *item = new QStandardItem(s);

    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmDaysItem(const alarm_event_t *ae)
{
    QStandardItem *item = new QStandardItem();
    item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    uint32_t wday = 0;
    QString days;
    time_t tick = (ae->flags & ALARM_EVENT_DISABLED) ? -1 : ae->trigger;

    if (ae->recurrence_tab)
        wday = ae->recurrence_tab->mask_wday;

    if (wday || tick == -1)
    {
        QStringList l = daysFromWday(wday);

        if (l.count() > 3)
            days = QStringList(l.mid(0, 3)).join(", ") + "\n" +
                    QStringList(l.mid(3)).join(", ");
        else
            days = l.join(", ");
    }
    else
    {
        QDateTime date = QDateTime::fromTime_t(tick);
        int daysRemain = QDateTime::currentDateTime().daysTo(date);

        if (daysRemain == 1)
            days = _("cloc_ti_start_tomorrow");
        else if (daysRemain < 8)
            days = QLocale().dayName(date.date().dayOfWeek(),
                                     QLocale::ShortFormat);
        else
            days = formatDateTime(tick, Date);
    }

    item->setData(wday, AlarmWdayRole);
    item->setText(days);
    item->setFont(QMaemo5Style::standardFont("SmallSystemFont"));

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    return item;
}

/* return alarm trigger time */
time_t QAlarmDialog::addAlarm(cookie_t cookie)
{
    QList<QStandardItem *> row;
    alarm_event_t *ae = alarmd_event_get(cookie);
    time_t tick = (ae->flags & ALARM_EVENT_DISABLED) ? -1 : ae->trigger;

    row << alarmCheckboxItem(cookie, ae)
        << alarmTimeItem(ae)
        << alarmTitleItem(ae)
        << alarmDaysItem(ae);

    alarm_event_delete(ae);

    model->appendRow(row);

    return tick;
}

void QAlarmDialog::addAlarms()
{
    cookie_t *list, *iter;
    time_t next = -1;
    QString nextAlarmDate;
    QString nextAlarmDay;

    model->removeRows(0, model->rowCount());
    /* get them */
    list = alarmd_event_query(0, 0, 0, 0, "worldclock_alarmd_id");

    if (*list)
    {
      label->hide();
      view->show();
    }
    else
    {
        view->hide();
        label->show();
    }

    for (iter = list; *iter != (cookie_t) 0; iter ++)
    {
        time_t t = addAlarm(*iter);

        if (t != -1 && (t < next || next == -1))
            next = t;
    }

    free(list);

    /* need to sort in reverse order :) */
    model->sort(1);
    model->sort(0, Qt::DescendingOrder);

    if (next != -1)
    {
        int days =
            QDateTime::currentDateTime().daysTo(QDateTime::fromTime_t(next));

        nextAlarmDate = _("cloc_ti_next") + " " + formatDateTime(next, Time);

        if (days == 1)
            nextAlarmDay = _("cloc_ti_start_tomorrow");
        else if (days < 8)
        {
            nextAlarmDay = QString(_("cloc_ti_start_day")).
                    replace("%s", formatDateTime(next, DayOfWeek));
        }
        else
            nextAlarmDay = formatDateTime(next, Date);
    }
    else
        nextAlarmDate = _("cloc_ti_start_no");

    emit nextAlarmChanged(QStringList() << nextAlarmDate << nextAlarmDay);
}

void QAlarmDialog::buttonClicked()
{
    QNewAlarmDialog d(this, false, "", QTime::currentTime(), 0,
                      true, 0);

    d.exec();
    addAlarms();
}

void QAlarmDialog::viewClicked(const QModelIndex &modelIndex)
{
    if (!view->itemValid())
    {
        view->selectionModel()->clearSelection();

        return;
    }

    int row = modelIndex.row();
    bool disabled = model->item(row, 0)->data(AlarmEnabledRole).toBool();
    uint seconds = model->item(row, 1)->data(AlarmDateTimeRole).toInt();
    QDateTime dt = QDateTime::fromTime_t(seconds);
    uint32_t wday = model->item(row, 3)->data(AlarmWdayRole).toUInt();
    QString text = model->item(row, 2)->text();
    cookie_t cookie = model->item(row, 0)->data(AlarmCookieRole).toLongLong();

    if (modelIndex.column() == 0)
    {
        /* toggle enabled state */
        alarm_event_t *ae = alarmd_event_get(cookie);

        /* do not try to enable an alarm that is in the past */
        if (ae->flags & ALARM_EVENT_DISABLED &&
                !ae->recurrence_tab &&
                dt < QDateTime::currentDateTime())
        {
            QNewAlarmDialog d(this, true, text, dt.time(), wday, false, cookie);

            alarm_event_delete(ae);
            d.exec();
        }
        else
        {
            ae->flags &= ~ALARM_EVENT_DISABLED;
            ae->flags |= disabled ? ALARM_EVENT_DISABLED : 0;
            cookie = alarmd_event_update(ae);
            alarm_event_delete(ae);
            model->item(row, 0)->setData((qlonglong)cookie, AlarmCookieRole);

            /* Make sure we've finished with all UI updates, looks ugly
             * otherwise
             */
            QCoreApplication::processEvents();

            if (!disabled)
            {
                ae = alarmd_event_get(cookie);
                showAlarmTimeBanner(ae->trigger);
                alarm_event_delete(ae);
            }
        }
    }
    else
    {
        QNewAlarmDialog d(this, true, text, dt.time(), wday, disabled, cookie);

        d.exec();
    }

    addAlarms();
}

void QAlarmTreeView::mousePressEvent(QMouseEvent *event)
{
    pressPos = event->pos();
    valid = true;
    QTreeView::mousePressEvent(event);
}

void QAlarmTreeView::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos() - pressPos;

    if (valid && sizeHintForRow(0) > 0 && abs(pos.x()) > sizeHintForRow(0))
    {
        selectionModel()->clearSelection();
        valid = false;
    }
}

void QAlarmTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    QTreeView::mouseReleaseEvent(event);
    selectionModel()->clearSelection();
}
