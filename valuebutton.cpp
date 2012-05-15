#include "valuebutton.h"
#include <QtMaemo5>
#include "osso-intl.h"
#include <QDebug>
// for dgettext
#include <libintl.h>
// for strftime
#include <time.h>
// for LOCALE
#include <locale.h>


ValueButton::ValueButton(QWidget * parent) : QMaemo5ValueButton(parent)
{
    setMinimumWidth(90);
    connect ( this, SIGNAL( clicked(bool) ), this, SLOT( clicked() ) );
}

void ValueButton::clicked()
{
    emit clicked(objectName());
}


static const char *getHildonTranslation(const char *string)
{
     setlocale (LC_ALL, "");
     const char *translation = ::dgettext("hildon-libs", string);
     if (qstrcmp(string, translation) == 0)
         return 0;
     return translation;
}

const char *hildonDateDayNameShort = getHildonTranslation("wdgt_va_date_day_name_short");

static QString formatHildonDate(const QDateTime &dt, const char *format)
{
     if (!format)
         return QString();

     char buf[255];
     struct tm tm = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

     if (!dt.date().isNull()) {
         tm.tm_wday = dt.date().dayOfWeek() % 7;
         tm.tm_mday = dt.date().day();
         tm.tm_mon = dt.date().month() - 1;
         tm.tm_year = dt.date().year() - 1900;
     }
     if (!dt.time().isNull()) {
         tm.tm_sec = dt.time().second();
         tm.tm_min = dt.time().minute();
         tm.tm_hour = dt.time().hour();
     }

     size_t resultSize = ::strftime(buf, sizeof(buf), format, &tm);
     if (!resultSize)
         return QString();

     return QString::fromUtf8(buf, resultSize);
}

void ValueButton::paintEvent(QPaintEvent *paint)
{
    QPushButton::paintEvent(paint);
    QPainter p(this);
    QRect r = this->rect();

    QFont f = p.font();
    f.setBold(false);
    f.setPointSize(18);
    p.setFont(f);
    p.setPen(QPen(QMaemo5Style::standardColor("DefaultTextColor")));

    p.drawText(20,0,this->width()-20,this->height(),Qt::AlignVCenter|Qt::AlignLeft,this->statusTip(), &r);

    QString name = this->valueText();
    

  if ( this->whatsThis() == "date" )
    {
	/* Calculate the daynames and remove the trailing comma for Polish and French point abv. */
        QString MondayNameShort = formatHildonDate(QDateTime::fromString("02.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 1 */
	QStringList sl = MondayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	MondayNameShort = sl.at(0);
        QString TuedayNameShort = formatHildonDate(QDateTime::fromString("03.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 2 */
	sl = TuedayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	TuedayNameShort = sl.at(0);
        QString WeddayNameShort = formatHildonDate(QDateTime::fromString("04.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 3 */
	sl = WeddayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	WeddayNameShort = sl.at(0);
        QString ThudayNameShort = formatHildonDate(QDateTime::fromString("05.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 4 */
	sl = ThudayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	ThudayNameShort = sl.at(0);
        QString FridayNameShort = formatHildonDate(QDateTime::fromString("06.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 5 */
	sl = FridayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	FridayNameShort = sl.at(0);
        QString SatdayNameShort = formatHildonDate(QDateTime::fromString("07.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 6 */
	sl = SatdayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	SatdayNameShort = sl.at(0);
        QString SundayNameShort = formatHildonDate(QDateTime::fromString("01.01.2012","dd.MM.yyyy"), hildonDateDayNameShort); /* 7 */
	sl = SundayNameShort.remove(QRegExp("(\\,|\\.)")).split(' ', QString::SkipEmptyParts);
	SundayNameShort = sl.at(0);
        //QTextStream(stdout) << sl.at(0);
        name.replace("1", MondayNameShort );
        name.replace("2", TuedayNameShort );
        name.replace("3", WeddayNameShort );
        name.replace("4", ThudayNameShort );
        name.replace("5", FridayNameShort );
        name.replace("6", SatdayNameShort );
        name.replace("7", SundayNameShort );
        name.replace("0", _("cloc_va_never") );
        name.replace("8", _("cloc_va_everyday") );
    }

    p.setPen(QPen(QMaemo5Style::standardColor("ActiveTextColor")));
    if ( this->whatsThis() != "alarm" )
        p.drawText(170,0,this->width()-170,this->height(),Qt::AlignVCenter|Qt::AlignLeft,name, &r);
    else
    {
        QFontMetrics fm(f);
        if ( QFileInfo(name).isFile() )
            name = QFileInfo(name).baseName();
        name = fm.elidedText(name, Qt::ElideRight, this->width()-250);
        p.drawText(240,0,this->width()-240,this->height(),Qt::AlignVCenter|Qt::AlignLeft,name, &r);
    }

    p.save();
    p.restore();

}
