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

#ifndef QMAEMO5WEEKDAYSPICKSELECTOR_H
#define QMAEMO5WEEKDAYSPICKSELECTOR_H

#include <QMaemo5AbstractPickSelector>
#include <QList>

struct QMaemo5WeekDaysPickWidget;

class QMaemo5WeekDaysPickSelector : public QMaemo5AbstractPickSelector
{
  Q_OBJECT

public:
    QMaemo5WeekDaysPickSelector(QObject *parent=0);
    QWidget* widget(QWidget* parent);
    QString currentValueText() const;
    QList<int> selectedDays();

public slots:
    void updateSelection(QList<int> new_sel);

private:
    QList<int> selection;
};

#endif // QMAEMO5WEEKDAYSPICKSELECTOR_H
