/*
 * This file is part of TimedSilencer.
 *
 *  TimedSilencer is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  TimedSilencer is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TimedSilencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PROFILEEVENT_H
#define PROFILEEVENT_H

#include <QTime>
#include <QVariant>
#include <QHash>
#include <QCryptographicHash>
#include <QStringList>
#include <QSettings>

enum WeekDay {NEVER, MON, TUE, WED, THU, FRI, SAT, SUN, EVERY_DAY};

class ProfileEvent : public QObject {
  Q_OBJECT
private:
  QByteArray id;

public:
  QTime from_time;
  QTime to_time;
  bool activated;
  QList<int> days;
  QList<long> alarmd_cookies;

  ProfileEvent() {
    // Activate as a default
    activated = true;
  }

  QByteArray getID() {
    if(id.isEmpty()) {
      QCryptographicHash hasher(QCryptographicHash::Md5);
      hasher.addData(from_time.toString().toLocal8Bit());
      hasher.addData(to_time.toString().toLocal8Bit());
      foreach(int i, days) hasher.addData(QByteArray::number(i));
      id = hasher.result();
    }
    //qDebug("getID(): %s", id.constData());
    return id;
  }

  QVariant save() const {
    QHash<QString, QVariant> m;
    m["from_time"] = from_time;
    m["to_time"] = to_time;
    m["activated"] = activated;
    QVariantList var_days;
    foreach(const int& day, days) var_days << day;
    m["days"] = var_days;
    QVariantList var_cookies;
    foreach(const long& c, alarmd_cookies) var_cookies << (qlonglong)c;
    m["alarmd_cookies"] = var_cookies;
    return m;
  }

  bool affectsCurrentTime() {
    if(!activated) return false;
    Q_ASSERT(!days.empty());
    if(days.empty()) days << NEVER;
    if(days.first() != EVERY_DAY && days.first() != NEVER) {
      // Check if the current week day is affected by this event
      if(!days.contains(QDate::currentDate().dayOfWeek())) return false;
    }
    // Ok, it is the right day, are we in the interval?
    bool in_silent_mode = false;
    QTime ctime = QTime::currentTime();
    if(from_time < to_time) {
      in_silent_mode = (ctime >= from_time && ctime < to_time);
    } else {
      // to_time is the next day
      in_silent_mode = (ctime >= from_time || (ctime < from_time && ctime < to_time));
    }
    return in_silent_mode;
  }

  static ProfileEvent* load(QVariant v) {
    QHash<QString, QVariant> m = v.toHash();
    ProfileEvent *pe = new ProfileEvent();
    pe->from_time = m.value("from_time").toTime();
    pe->to_time = m.value("to_time").toTime();
    pe->activated = m.value("activated").toBool();
    QVariantList var_days = m.value("days").toList();
    foreach(const QVariant& var_day, var_days) pe->days << var_day.toInt();
    QVariantList var_cookies = m.value("alarmd_cookies").toList();
    foreach(const QVariant& var_c, var_cookies) pe->alarmd_cookies << var_c.toLongLong();
    return pe;
  }

  static QString formatDays(QList<int> selection) {
    qDebug("CurrentValueText() called");
    if(selection.isEmpty() || selection.contains(NEVER)) {
      return tr("Never");
    }
    if(selection.contains(EVERY_DAY)) {
      return tr("Every day");
    }
    QStringList selectedDays;
    foreach(const int &i, selection) {
      selectedDays << QDate::shortDayName(i);
    }
    return selectedDays.join(", ");
  }

  static ProfileEvent* findByID(QByteArray myid) {
    QSettings settings("TimedSilencer", "TimedSilencer");
    QHash<QString, QVariant> events = settings.value("events").toHash();
    if(events.contains(myid)) {
      return load(events.value(myid));
    }
    return 0;
  }

  static void clearCookies(QByteArray myid) {
    QSettings settings("TimedSilencer", "TimedSilencer");
    QHash<QString, QVariant> events = settings.value("events").toHash();
    //Q_ASSERT(events.contains(myid));
    if(events.contains(myid)) {
      qDebug("Clearing event cookies in QSettings");
      ProfileEvent *pe = load(events.value(myid));
      pe->alarmd_cookies.clear();
      events[myid] = pe->save();
      settings.setValue("events", events);
      delete pe;
    }
  }

  static void setCookies(QByteArray myid, QList<long> cookies) {
    QSettings settings("TimedSilencer", "TimedSilencer");
    QHash<QString, QVariant> events = settings.value("events").toHash();
    Q_ASSERT(events.contains(myid));
    if(events.contains(myid)) {
      qDebug("Setting event cookies in QSettings");
      ProfileEvent *pe = load(events.value(myid));
      Q_ASSERT(pe->alarmd_cookies.empty());
      pe->alarmd_cookies = cookies;
      events[myid] = pe->save();
      settings.setValue("events", events);
      delete pe;
    }
  }

  static void setStatus(QByteArray myid, bool status) {
    QSettings settings("TimedSilencer", "TimedSilencer");
    QHash<QString, QVariant> events = settings.value("events").toHash();
    Q_ASSERT(events.contains(myid));
    if(events.contains(myid)) {
      qDebug("Setting event status in QSettings");
      ProfileEvent *pe = load(events.value(myid));
      pe->activated = status;
      events[myid] = pe->save();
      settings.setValue("events", events);
      delete pe;
    }
  }

};

#endif // PROFILEEVENT_H
