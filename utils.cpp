#include "utils.h"
#include <libalarm.h>
#include "gconfitem.h"

#include <QDateTime>
#include <QLocale>
#include <QProcess>
#include <QMaemo5InformationBox>
#include <QCoreApplication>

#include <glib.h>

bool is24HoursClock()
{
    return GConfItem("/apps/clock/time-format").value().toBool();
}

QString formatDateTime(const time_t tick, DateTime what)
{
    char buf[256];
    size_t len = 0;
    const struct tm *t = localtime(&tick);

    switch (what)
    {
        case Time:
        {
            if (is24HoursClock())
                len = _strftime(buf, sizeof(buf), "wdgt_va_24h_time", t);
            else
            {
                if (t->tm_hour > 11)
                    len = _strftime(buf, sizeof(buf), "wdgt_va_12h_time_pm", t);
                else
                    len = _strftime(buf, sizeof(buf), "wdgt_va_12h_time_am", t);
            }

            break;
        }
        case Hour12:
        {
            len = _strftime(buf, sizeof(buf), "wdgt_va_12h_hours", t);
            break;
        }
        case Time12:
        {
            return formatDateTime(tick, Hour12) + ":" +
                    formatDateTime(tick, Minutes);
        }
        case TimeSeconds:
        {
            len = _strftime(buf, sizeof(buf), "wdgt_va_full_24h_time", t);
            break;
        }
        case Minutes:
        {
            len = _strftime(buf, sizeof(buf), "wdgt_va_minutes", t);
            break;
        }
        case MinutesSeconds:
        {
            len = _strftime(buf, sizeof(buf), "wdgt_va_minutes_seconds", t);
            break;
        }
        case amPm:
        {
            if (t->tm_hour > 11)
                len = _strftime(buf, sizeof(buf), "wdgt_va_pm", t);
            else
                len = _strftime(buf, sizeof(buf), "wdgt_va_am", t);

            break;
        }
        case DayOfWeek:
            len = _strftime(buf, sizeof(buf), "wdgt_va_week", t);
            break;
        case Date:
            len = _strftime(buf, sizeof(buf), "wdgt_va_date", t);
            break;
        case FullDateShort:
        {
            QString rv;
            len = _strftime(buf, sizeof(buf), "wdgt_va_week_name_short", t);
            rv = QString::fromUtf8(buf, len) + ", ";
            len = _strftime(buf, sizeof(buf), "wdgt_va_day_numeric", t);
            rv += QString::fromUtf8(buf, len) + " ";
            len = _strftime(buf, sizeof(buf), "wdgt_va_month_name_short", t);
            rv += QString::fromUtf8(buf, len) + " ";
            len = _strftime(buf, sizeof(buf), "wdgt_va_year", t);
            rv += QString::fromUtf8(buf, len);

            return rv;
        }
        case FullDateLong:
            len = _strftime(buf, sizeof(buf), "wdgt_va_date_long", t);
            break;
    }

    return QString::fromUtf8(buf, len);
}

QStringList daysFromWday(uint32_t mask_wday)
{
    QStringList rv;
    QLocale locale;

    if (!mask_wday)
        rv << _("cloc_va_never");
    else if (mask_wday == ALARM_RECUR_WDAY_ALL)
        rv << _("cloc_va_everyday");
    else
    {
        for (int i = 1; i < 8; i ++)
        {
            /* Sunday is 1 << 0 */
            if (mask_wday & (1 << (i % 7)))
                rv << locale.dayName(i, QLocale::ShortFormat);
        }
    }

    return rv;
}
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define HOURS_PER_DAY 24
#define SECONDS_PER_HOUR (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY (HOURS_PER_DAY * SECONDS_PER_HOUR)

/* FIXME - try to not use glib */
static QString formatAlarmTimeBanner(time_t tick)
{
    time_t now = QDateTime::currentDateTime().toTime_t();
    struct tm t;
    char buf[256];
    gchar *d, *h, *s;
    QString rv;
    const char *cloc_notify_alarm_set = "cloc_notify_alarm_set";

    intl("osso-clock");
    memset(&t, 0, sizeof(t));

    if (now <= tick)
    {
        time_t diff = tick - now;
        if (diff < SECONDS_PER_DAY)
        {
            t.tm_hour = (diff % SECONDS_PER_DAY) / SECONDS_PER_HOUR;
            t.tm_min = (diff % SECONDS_PER_HOUR) / SECONDS_PER_MINUTE;
        }
        else
        {
            int days = diff / SECONDS_PER_DAY;
            int hours = (diff % SECONDS_PER_DAY) / SECONDS_PER_HOUR;
            const char *cloc_va_amount_day = "cloc_va_amount_day";
            const char *cloc_va_amount_hour2 = "cloc_va_amount_hour2";
            const char *cloc_notify_alarm_set_days =
                    "cloc_notify_alarm_set_days";

            d = g_strdup_printf(dngettext(NULL, cloc_va_amount_day,
                                          "cloc_va_amount_days", days),
                                days);
            h = g_strdup_printf(dngettext(NULL, cloc_va_amount_hour2,
                                          "cloc_va_amount_hours2", hours), d,
                                hours);
            s = g_strdup_printf(dgettext(NULL, cloc_notify_alarm_set_days), h);
            g_free(d);
            g_free(h);

            rv = QString::fromUtf8(s);
            g_free(s);

            goto out;
        }
    }

    _strftime(buf, sizeof(buf), "wdgt_va_24h_time", &t);
    s = g_strdup_printf(dgettext(NULL, cloc_notify_alarm_set), buf);
    rv = QString::fromUtf8(s);
    g_free(s);

out:
    return rv;
}

void showAlarmTimeBanner(time_t tick)
{
    QMaemo5InformationBox::information(NULL, formatAlarmTimeBanner(tick));
    QCoreApplication::processEvents();
}

QString formatTimeMarkup(time_t tick, const QString &timeSize,
                         const QString &amPmSize, const QString &align,
                         bool seconds)
{
    QString rv;

    if (is24HoursClock())
    {
        QString time;

        if (seconds)
            time = formatDateTime(tick, TimeSeconds);
        else
            time = formatDateTime(tick, Time);

        rv = "<p align=" + align + " style=" + timeSize +
                "margin-top:0px;margin-bottom:0px;>" + time + "</p>";
    }
    else
    {
        QString tmp = formatDateTime(tick, Hour12) + ":";

        tmp += formatDateTime(tick, seconds ? MinutesSeconds : Minutes);
        rv = "<p align=" + align + " style=" + timeSize + ">" + tmp;
        rv += "<span style=" + amPmSize + "> ";
        rv += formatDateTime(tick, amPm) + "</span></p>";
    }

    return rv;
}
