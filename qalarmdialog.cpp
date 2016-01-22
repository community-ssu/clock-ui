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
#include <QHeaderView>

#include "utils.h"

enum {
    AlarmEnabledRole = Qt::UserRole + 1,
    AlarmCookieRole,
    AlarmWdayRole,
    AlarmDateTimeRole
};

enum {
    tickColumn,
    timeColumn,
    sepCol1,
    titleColumn,
    sepCol2,
    daysColumn,
    lastColumn
};

#define SEPARATOR_WIDTH 16

QAlarmDialog::QAlarmDialog(QWidget *parent) :
    QDialog(parent),
    label(new QLabel),
    button(new QPushButton()),
    view(new QAlarmTreeView()),
    model(new QStandardItemModel(0, lastColumn, view))
{
    setWindowTitle(_("cloc_ti_alarms"));
    setAttribute(Qt::WA_Maemo5StackedWindow);
    setWindowFlags(Qt::Window);

    /* model/view */
    model->setSortRole(AlarmEnabledRole);

    view->setModel(model);
    view->setColumnWidth(tickColumn, 64);
    view->setColumnWidth(sepCol1, SEPARATOR_WIDTH);
    view->setColumnWidth(sepCol2, SEPARATOR_WIDTH);
    view->header()->setStretchLastSection(false);
    view->header()->setResizeMode(timeColumn, QHeaderView::ResizeToContents);
    view->header()->setResizeMode(titleColumn, QHeaderView::Stretch);
    view->header()->setResizeMode(daysColumn, QHeaderView::ResizeToContents);
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
                                             const alarm_event_t *ae) const
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

void QAlarmDialog::alarmTimeItem(const alarm_event_t *ae,
                                 QStandardItem *item) const
{
    time_t tick = ae->snooze_total;
    QAlarmTimeLabel *label = new QAlarmTimeLabel(view, item);
    QString color;

    label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if (!tick)
        tick = ae->trigger;

    if (ae->flags & ALARM_EVENT_DISABLED)
    {
        color = "color:" +
                QMaemo5Style::standardColor("SecondaryTextColor").name() + ";";
    }

    label->setText(formatTimeMarkup(tick, "font-size:x-large;" + color,
                                    "font-size:small;" + color, "right",
                                    false));
    /* needed so sorting to work */
    item->setData(QDateTime::fromTime_t(tick).time());
    item->setData((int)tick, AlarmDateTimeRole);
    view->setIndexWidget(item->index(), label);
    connect(label, SIGNAL(clicked(QModelIndex)),
            this, SLOT(viewClicked(QModelIndex)));
}

QStandardItem *QAlarmDialog::alarmTitleItem(const alarm_event_t *ae) const
{
    const char *s = alarm_event_get_message(ae);

    if(!s)
        s = alarm_event_get_title(ae);

    QStandardItem *item = new QStandardItem(QString::fromUtf8(s));

    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    if (ae->flags & ALARM_EVENT_DISABLED)
        item->setData(QMaemo5Style::standardColor("SecondaryTextColor"),
                      Qt::ForegroundRole);

    return item;
}

QStandardItem *QAlarmDialog::alarmDaysItem(const alarm_event_t *ae) const
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
time_t QAlarmDialog::_addAlarm(cookie_t cookie)
{
    QList<QStandardItem *> row;
    alarm_event_t *ae = alarmd_event_get(cookie);
    time_t tick = (ae->flags & ALARM_EVENT_DISABLED) ? -1 : ae->trigger;
    QStandardItem *item = new QStandardItem;

    row << alarmCheckboxItem(cookie, ae)
        << item
        << new QStandardItem
        << alarmTitleItem(ae)
        << new QStandardItem
        << alarmDaysItem(ae);

    model->appendRow(row);
    alarmTimeItem(ae, item);
    alarm_event_delete(ae);

    return tick;
}

void QAlarmDialog::sort()
{
    /* need to sort in reverse order :) */
    model->sort(timeColumn);
    model->sort(tickColumn, Qt::DescendingOrder);
}

void QAlarmDialog::addAlarms()
{
    cookie_t *list, *iter;

    model->removeRows(0, model->rowCount());
    /* get them */
    list = alarmd_event_query(0, 0, 0, 0, "worldclock_alarmd_id");

    for (iter = list; *iter != (cookie_t) 0; iter ++)
        _addAlarm(*iter);

    free(list);
    sort();

    signalNextAlarm();
}

void QAlarmDialog::signalNextAlarm()
{
    QTimer::singleShot(0, this, SLOT(getNextAlarm()));
}

void QAlarmDialog::getNextAlarm() const
{
    cookie_t *list, *iter;
    time_t next = -1;
    QString nextAlarmDate;
    QString nextAlarmDay;

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
        time_t t;

        alarm_event_t *ae = alarmd_event_get(*iter);
        t = (ae->flags & ALARM_EVENT_DISABLED) ? -1 : ae->trigger;
        alarm_event_delete(ae);

        if (t != -1 && (t < next || next == -1))
            next = t;
    }

    free(list);

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

void QAlarmDialog::addAlarm(cookie_t cookie)
{
    _addAlarm(cookie);
    sort();
    signalNextAlarm();
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
    bool disabled =
            model->item(row, tickColumn)->data(AlarmEnabledRole).toBool();
    uint seconds =
            model->item(row, timeColumn)->data(AlarmDateTimeRole).toInt();
    QDateTime dt = QDateTime::fromTime_t(seconds);
    uint32_t wday = model->item(row, daysColumn)->data(AlarmWdayRole).toUInt();
    QString text = model->item(row, titleColumn)->text();
    cookie_t cookie =
            model->item(row, tickColumn)->data(AlarmCookieRole).toLongLong();
    cookie_t newcookie = -1;

    if (modelIndex.column() == tickColumn)
    {
        /* toggle enabled state */
        alarm_event_t *ae = alarmd_event_get(cookie);

        if (ae->flags & ALARM_EVENT_DISABLED &&
                !wday &&
                dt < QDateTime::currentDateTime())
        {
            ae->alarm_time = -1;
            ae->alarm_tm.tm_hour = dt.time().hour();
            ae->alarm_tm.tm_min = dt.time().minute();
        }

        ae->flags &= ~ALARM_EVENT_DISABLED;
        ae->flags |= disabled ? ALARM_EVENT_DISABLED : 0;
        newcookie = alarmd_event_update(ae);
        alarm_event_delete(ae);

        if (!disabled)
        {
            ae = alarmd_event_get(newcookie);
            showAlarmTimeBanner(ae->trigger);
            alarm_event_delete(ae);
        }
    }
    else
    {
        QNewAlarmDialog d(this, true, text, dt.time(), wday, disabled, cookie);

        if (d.exec() == QDialog::Accepted)
        {
            newcookie = d.realcookie;

            if (newcookie == -1)
            {
                /* deleted */
                setUpdatesEnabled(false);
                model->removeRow(row);
                sort();
                signalNextAlarm();
                setUpdatesEnabled(true);
            }

        }
    }

    if (newcookie != -1)
    {
        model->removeRow(row);
        addAlarm(newcookie);
    }
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
