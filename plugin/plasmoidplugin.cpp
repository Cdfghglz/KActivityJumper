/*
    Copyright (c) 2016 Carlos López Sánchez <musikolo{AT}hotmail[DOT]com>

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

#include "plasmoidplugin.h"

#include <QtQml>
#include <QDebug>

void PlasmoidPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.private.activityjumper"));

    qmlRegisterType<DBusInterface>(uri, 1, 0, "ActivityJumper");
}

QDBusInterface * DBusInterface::initItfFromStringL(QStringList interfaceStringList) {
	QDBusInterface *activityListInterface = new QDBusInterface(
			interfaceStringList[0], interfaceStringList[1], interfaceStringList[2],
			QDBusConnection::sessionBus(),
			this);
	return activityListInterface;
}

int DBusInterface::jumpBack() {
	QDBusInterface* activityJumperItf = initItfFromStringL(ACTIVITY_JUMPER_ITF_STRING);
	QDBusMessage response = activityJumperItf->call("jumpBack");
	if (response.type() != QDBusMessage::ErrorMessage) {
		return 0;
	} else {
		qDebug() << response.errorName();
		return 1;
	}
}
