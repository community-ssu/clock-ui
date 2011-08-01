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

#include <QDate>

#include "qmaemo5weekdayspickselector.h"
#include "qmaemo5weekdayspickwidget.h"
#include "profileevent.h"

QMaemo5WeekDaysPickSelector::QMaemo5WeekDaysPickSelector(QObject *parent): QMaemo5AbstractPickSelector(parent)
{
}


QWidget* QMaemo5WeekDaysPickSelector::widget(QWidget* parent) {
  QMaemo5WeekDaysPickWidget *pick_widget = new QMaemo5WeekDaysPickWidget(parent);
  pick_widget->setSelected(selection);
  connect(pick_widget, SIGNAL(selectedDays(QList<int>)), this, SLOT(updateSelection(QList<int>)));
  return pick_widget;
}

QString QMaemo5WeekDaysPickSelector::currentValueText() const {
  qDebug("CurrentValueText() called");
  return ProfileEvent::formatDays(selection);
}

void QMaemo5WeekDaysPickSelector::updateSelection(QList<int> new_sel) {
  qDebug("New selection size: %d", new_sel.size());
  selection = new_sel;
  emit selected(currentValueText());
}

QList<int> QMaemo5WeekDaysPickSelector::selectedDays() {
  if(selection.empty()) selection << NEVER;
  return selection;
}
