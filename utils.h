#ifndef UTILS_H
#define UTILS_H

#include <osso-intl.h>
#include <time.h>
#include <stdint.h>

#include <QStringList>

typedef enum {
    Time,
    Hour12,
    Time12,
    TimeSeconds,
    Minutes,
    MinutesSeconds,
    amPm,
    DayOfWeek,
    Date,
    FullDateShort,
    FullDateLong
} DateTime;

QString formatDateTime(time_t trigger, DateTime what);

QStringList daysFromWday(uint32_t mask_wday);

void showAlarmTimeBanner(time_t tick);

static inline size_t _strftime(char *s, size_t max, const char* format,
                                     const struct tm *t)
{
    return strftime(s, max, dgettext("hildon-libs", format), t);
}

bool is24HoursClock();
QString formatTimeMarkup(time_t tick, const QString &timeSize,
                         const QString &amPmSize, const QString &align,
                         bool seconds);

#endif // UTILS_H
