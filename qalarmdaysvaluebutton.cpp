#include "qalarmdaysvaluebutton.h"
#include "osso-intl.h"
#include "utils.h"

#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QLocale>
#include <QApplication>
#include <QDesktopWidget>

#include <QDebug>

#define CHECK_COLUMN 2

QMaemo5DaysPickSelector::QMaemo5DaysPickSelector(QObject *parent) :
    QMaemo5AbstractPickSelector(parent),
    d(0),
    view(new QTreeView(0)),
    model(new QStandardItemModel(0, 3, view))
{
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setIndentation(0);
    view->setIconSize(QSize(48, 48));
    view->setUniformRowHeights(true);
    view->setHeaderHidden(true);
    view->setModel(model);

    QLocale locale;

    appendDay(_("cloc_va_never"));

    for (int i = 1; i < 8; i ++)
        appendDay(locale.dayName(i, QLocale::LongFormat));

    appendDay(_("cloc_va_everyday"));

    updateSelection();

    if (view->sizeHintForRow(0) > 0)
    {
        int sh = view->sizeHintForRow(0);

        view->setColumnWidth(0, sh);
        view->setColumnWidth(2, sh);
    }

    connect(view, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT(viewClicked(const QModelIndex &)));
}

void QMaemo5DaysPickSelector::appendDay(const QString &name)
{
    QList<QStandardItem *> row;
    QStandardItem *item;

    item = new QStandardItem();
    item->setEditable(false);
    row << item;

    item = new QStandardItem(name);
    item->setTextAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    item->setEditable(false);
    row << item;

    item = new QStandardItem();
    item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    item->setEditable(false);
    row << item;

    model->appendRow(row);}

void QMaemo5DaysPickSelector::checkRow(int row, bool check)
{
    QStandardItem *item = model->item(row, CHECK_COLUMN);

    if (check)
        item->setIcon(QIcon::fromTheme("widgets_tickmark_list"));
    else
        item->setIcon(QIcon());
}

void QMaemo5DaysPickSelector::updateSelection()
{
    QItemSelection iss, ids;

    int rows = model->rowCount();
    int i;

    checkRow(0, !d);

    if (!d)
    {
        for (i = 1; i < rows; i ++)
            checkRow(i, false);

        iss << QItemSelection(model->index(0, 0),
                              model->index(0, CHECK_COLUMN));
        ids << QItemSelection(model->index(1, 0),
                              model->index(rows - 1, CHECK_COLUMN));
    }
    else
    {
        ids << QItemSelection(model->index(0, 0),
                              model->index(0, CHECK_COLUMN));

        for (i = 1; i < rows - 1; i ++)
        {
            bool checked =  d & (1 << (i - 1));
            checkRow(i, checked);
            if (d != ALARM_RECUR_WDAY_ALL)
            {
                QItemSelection is(model->index(i, 0),
                                  model->index(i, CHECK_COLUMN));
                if (checked)
                    iss << is;
                else
                    ids << is;
            }
        }

        if (d == ALARM_RECUR_WDAY_ALL)
        {
            checkRow(rows - 1 , true);
            iss << QItemSelection(model->index(1, 0),
                                  model->index(rows - 1, CHECK_COLUMN));
        }
        else
        {
            checkRow(rows - 1 , false);
            ids << QItemSelection(model->index(rows - 1, 0),
                                  model->index(rows - 1, CHECK_COLUMN));
        }
    }

    view->selectionModel()->select(ids, QItemSelectionModel::Deselect);
    view->selectionModel()->select(iss, QItemSelectionModel::Select);
}

QMaemo5DaysPickSelector::~QMaemo5DaysPickSelector()
{
    delete view;
}

void QMaemo5DaysPickSelector::viewClicked(const QModelIndex &index)
{
    int row = index.row();

    if (!row)
        d = 0;
    else if (row == (model->rowCount() - 1))
        d = ALARM_RECUR_WDAY_ALL;
    else
    {
        int bit = (1 << (row - 1));
        d = (d & bit) ? (d & ~bit) : (d | bit);
    }

    updateSelection();
}

QWidget *QMaemo5DaysPickSelector::widget(QWidget *parent)
{
    QWidget *dialog = new DaysPickerDialog(this, parent);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (QAbstractButton *pb = qobject_cast<QAbstractButton *>(parent))
            dialog->setWindowTitle(pb->text());

    return dialog;
}

QAlarmDaysValueButton::QAlarmDaysValueButton(QWidget *parent) :
    QAlarmValueButton(parent),
    selector(new QMaemo5DaysPickSelector(this))
{
    setText(_("cloc_fi_repeat"));
    setPickSelector(selector);
    connect(selector, SIGNAL(selected(QString)),
            this, SLOT(daysSelected(QString)));
}

QString QAlarmDaysValueButton::valueText() const
{
    return daysFromWday(days()).join(",");
}

DaysPickerDialog::~DaysPickerDialog()
{
    /* give back the view */
    if (p->view)
        p->view->setParent(0);
}

void DaysPickerDialog::paintEvent(QPaintEvent *e)
{
    QDialog::paintEvent(e);
    int sh = p->view->sizeHintForRow(0);

    if (sh > 0)
        p->view->setColumnWidth(1, p->view->width() - 2 * sh - 16);
}

void DaysPickerDialog::accepted()
{
    emit (p->selected(p->currentValueText()));
    QDialog::accept();
}

void DaysPickerDialog::orientationChanged()
{
    QRect geometry = QApplication::desktop()->screenGeometry();
    QSizePolicy sp;
    QLayout *layout;

    setUpdatesEnabled(false);
    if (geometry.width() < geometry.height())
    {
        /* portrait */
        layout = this->layout();
        if (layout)
        {
            layout->removeWidget(box);
            layout->removeWidget(p->view);
            delete layout;
        }
        layout = new QVBoxLayout(this);
        sp = QSizePolicy(QSizePolicy::QSizePolicy::MinimumExpanding,
                         QSizePolicy::Fixed);
        if (p->view->sizeHintForRow(0) > 0)
            p->view->setMinimumHeight(p->view->sizeHintForRow(0) * 8);
    }
    else
    {
        layout = this->layout();
        if (layout)
        {
            layout->removeWidget(box);
            layout->removeWidget(p->view);
            delete layout;
        }
        layout = new QHBoxLayout(this);
        sp = QSizePolicy(QSizePolicy::Fixed,
                         QSizePolicy::QSizePolicy::MinimumExpanding);

        if (p->view->sizeHintForRow(0) > 0)
            p->view->setMinimumHeight(p->view->sizeHintForRow(0) * 5);
    }

    box->setSizePolicy(sp);
    layout->setContentsMargins(16, 0, 16, 8);
    layout->addWidget(p->view);
    layout->addWidget(box);
    setUpdatesEnabled(true);
}

DaysPickerDialog::DaysPickerDialog(QMaemo5DaysPickSelector *picker,
                                   QWidget *parent) :
    QDialog(parent), p(picker)
{

    box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    box->setOrientation(Qt::Vertical);
    box->setCenterButtons(false);

    connect(box, SIGNAL(accepted()), this, SLOT(accepted()));

    connect(QApplication::desktop(), SIGNAL(resized(int)),
            this, SLOT(orientationChanged()));

    orientationChanged();
}
