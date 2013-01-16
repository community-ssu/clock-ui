/*
    osso-intl.h - OSSO intl
    Copyright (C) 2011  Pali Rohár <pali.rohar@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef OSSO_INTL_H
#define OSSO_INTL_H

#include <QByteArray>
#include <QString>

#include <libintl.h>

static inline void intl(const char * package) {

	setlocale(LC_ALL, "");
	bindtextdomain(package, "/usr/share/locale");
	bind_textdomain_codeset(package, "UTF-8");
	textdomain(package);

}

static inline const QString _(const char * str) {

	return QString::fromUtf8(gettext(str)).replace("%%", "%");

}

static inline const QString _(const QString &str) {

	return _(str.toUtf8().data());

}

#endif
