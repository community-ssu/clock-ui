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

#ifndef QMAEMO5WEEKDAYSPICKWIDGET_H
#define QMAEMO5WEEKDAYSPICKWIDGET_H

#include <QDialog>
#include <QModelIndex>

#include "profileevent.h"

struct QListView;
struct QDialogButtonBox;

class QMaemo5WeekDaysPickWidget : public QDialog
{
    Q_OBJECT
public:
    explicit QMaemo5WeekDaysPickWidget(QWidget *parent = 0);

signals:
    void selectedDays(QList<int> days);

public slots:
    void ensureConsistentSelection(QModelIndex index);
    void emitSelectionAndClose();
    void setSelected(QList<int> days);

private:
  QListView *daysList;
  QDialogButtonBox *button_box;
};

#endif // QMAEMO5WEEKDAYSPICKWIDGET_H
