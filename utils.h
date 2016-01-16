#ifndef UTILS_H
#define UTILS_H

#include <osso-intl.h>
#include <time.h>
#include <stdint.h>

#include <QStringList>

typedef enum {
    Time,
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

#endif // UTILS_H
