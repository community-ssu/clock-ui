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

#include <QListView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QSpacerItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QItemSelectionModel>
#include "osso-intl.h"
#include "qmaemo5weekdayspickwidget.h"
//#include "checklistdelegate.h"

QMaemo5WeekDaysPickWidget::QMaemo5WeekDaysPickWidget(QWidget *parent) :
    QDialog(parent)
{

  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("Repeat"));
  QHBoxLayout *hLayout = new QHBoxLayout(this);
  QVBoxLayout *vLayoutL = new QVBoxLayout;
  QStandardItemModel *model = new QStandardItemModel(9, 1);
  model->setItem(NEVER, 0, new QStandardItem(_("cloc_va_never")));
  intl("modest-nokiamessaging-plugin");
  model->setItem(MON, 0, new QStandardItem(_("nm_fi_day_monday")));
  model->setItem(TUE, 0, new QStandardItem(_("nm_fi_day_tuesday")));
  model->setItem(WED, 0, new QStandardItem(_("nm_fi_day_wednesday")));
  model->setItem(THU, 0, new QStandardItem(_("nm_fi_day_thursday")));
  model->setItem(FRI, 0, new QStandardItem(_("nm_fi_day_friday")));
  model->setItem(SAT, 0, new QStandardItem(_("nm_fi_day_saturday")));
  model->setItem(SUN, 0, new QStandardItem(_("nm_fi_day_sunday")));
  intl("osso-clock");

  model->setItem(EVERY_DAY, 0, new QStandardItem(_("cloc_va_everyday")));
  daysList = new QListView;
  daysList->setModel(model);
  //daysList->setItemDelegate(new CheckListDelegate);
  connect(daysList, SIGNAL(activated(QModelIndex)), this, SLOT(ensureConsistentSelection(QModelIndex)));
  // Select NEVER item
  daysList->selectionModel()->select(daysList->model()->index(NEVER, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
  // Height hint
  if (daysList->sizeHintForRow(0)>0)
    daysList->setMinimumHeight(daysList->sizeHintForRow(0) * 5);
  daysList->setSelectionMode(QAbstractItemView::MultiSelection);
  daysList->setSelectionBehavior(QAbstractItemView::SelectRows);
  vLayoutL->addWidget(daysList);
  hLayout->addLayout(vLayoutL);
  QVBoxLayout *vLayoutR = new QVBoxLayout;
  vLayoutR->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));
  button_box = new QDialogButtonBox(Qt::Vertical);
  QPushButton *done_btn = new QPushButton(tr("Done"));
  connect(done_btn, SIGNAL(clicked()), this, SLOT(emitSelectionAndClose()));
  done_btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  button_box->addButton(done_btn, QDialogButtonBox::ActionRole);
  vLayoutR->addWidget(button_box);
  hLayout->addLayout(vLayoutR);
  qDebug("QMaemo5WeekDaysPickWidget constructed");
}

void QMaemo5WeekDaysPickWidget::setSelected(QList<int> days) {
  if(days.empty()) {
    daysList->selectionModel()->select(daysList->model()->index(NEVER, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    return;
  }
  daysList->selectionModel()->clearSelection();
  qDebug("setSelected, %d items", days.size());
  foreach(const int &d, days) {
    daysList->selectionModel()->select(daysList->model()->index(d, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    if(d == EVERY_DAY)
      ensureConsistentSelection(daysList->model()->index(d, 0));
  }
}

void QMaemo5WeekDaysPickWidget::emitSelectionAndClose() {
  qDebug("in emitSelectionAndClose()");
  QList<int> selected_rows;
  QModelIndexList selected_indexes = daysList->selectionModel()->selectedRows();
  foreach(QModelIndex index, selected_indexes) {
    // Insert sort
    int i = 0;
    const int row = index.row();
    while(i < selected_rows.size() && selected_rows.at(i) < row) ++i;
    selected_rows.insert(i, row);
  }
  emit selectedDays(selected_rows);
  close();
}

void QMaemo5WeekDaysPickWidget::ensureConsistentSelection(QModelIndex index) {

  qDebug("Received a click");
  switch(index.row()) {
  case NEVER:
    if(!daysList->selectionModel()->isSelected(index)) {
      // Prevent unselect
      daysList->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      return;
    }
    daysList->selectionModel()->clearSelection();
    daysList->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    break;
  case EVERY_DAY:
    if(!daysList->selectionModel()->isSelected(index)) {
      // Prevent unselect
      daysList->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
      return;
    }
    if(daysList->selectionModel()->isRowSelected(NEVER, daysList->rootIndex())) {
      // Unselect NEVER item
      daysList->selectionModel()->select(daysList->model()->index(NEVER, 0), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
    // Select all days
    for(int i=MON; i<EVERY_DAY; ++i) {
      daysList->selectionModel()->select(daysList->model()->index(i, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    break;
  default:
    if(daysList->selectionModel()->isRowSelected(NEVER, daysList->rootIndex())) {
      // Unselect NEVER item
      daysList->selectionModel()->select(daysList->model()->index(NEVER, 0), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
    if(!daysList->selectionModel()->isSelected(index)) {
      if(daysList->selectionModel()->isRowSelected(EVERY_DAY, daysList->rootIndex())) {
        // A Work day was unselected, unselect EVERY_DAY item
        daysList->selectionModel()->select(daysList->model()->index(EVERY_DAY, 0), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
      }
      if(!daysList->selectionModel()->hasSelection()) {
        // Select NEVER item
        daysList->selectionModel()->select(daysList->model()->index(NEVER, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
      }
    } else {
      // If all days are selected, select EVERY_DAY
      bool all_days_selected = true;
      for(int i=MON; i<EVERY_DAY; ++i) {
        if(!daysList->selectionModel()->isRowSelected(i, daysList->rootIndex())) {
          all_days_selected = false;
          break;
        }
      }
      if(all_days_selected)
        daysList->selectionModel()->select(daysList->model()->index(EVERY_DAY, 0), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
  }
}
