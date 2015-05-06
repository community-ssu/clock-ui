#ifndef UTILS_H
#define UTILS_H

#include <time.h>
#include <QStringList>

typedef enum {
    Time,
    DayOfWeek,
    Date,
    FullDateShort
} DateTime;

QString formatDateTime(time_t trigger, DateTime what);

QStringList daysFromWday(uint32_t mask_wday);

void showAlarmTimeBanner(time_t tick);
#endif // UTILS_H
