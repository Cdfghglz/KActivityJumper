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
#include <kwindowsystem.h>


void PlasmoidPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.private.activityjumper"));

    qmlRegisterType<ActivityJumperItf>(uri, 1, 0, "ActivityJumper");

}

ActivityJumperItf::ActivityJumperItf(QObject *parent) : QObject(parent) {
	connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &ActivityJumperItf::desktopChanged);
}
ActivityJumperItf::~ActivityJumperItf() {
}

QDBusInterface * ActivityJumperItf::initItfFromStringL(QStringList interfaceStringList) {
	QDBusInterface *activityListInterface = new QDBusInterface(
			interfaceStringList[0], interfaceStringList[1], interfaceStringList[2],
			QDBusConnection::sessionBus(),
			this);
	return activityListInterface;
}

int ActivityJumperItf::jumpBack() {
	QDBusInterface* activityJumperItf = initItfFromStringL(ACTIVITY_JUMPER_ITF_STRING);
	QDBusMessage response = activityJumperItf->call("jumpBack");
	if (response.type() != QDBusMessage::ErrorMessage) return 0;
	else {
		qDebug() << "jumpBack: " << response.errorName();
		return 1;
	}
}

int ActivityJumperItf::changePinState() {
	QDBusInterface* activityJumperItf = initItfFromStringL(ACTIVITY_JUMPER_ITF_STRING);
	QDBusMessage response = activityJumperItf->call("changePinState");
	if (response.type() != QDBusMessage::ErrorMessage) return 0;
	else {
		qDebug() << "changePinState: " << response.errorName();
		return 1;
	}
}

int ActivityJumperItf::getPinState() {
	QDBusInterface *activityJumperItf = initItfFromStringL(ACTIVITY_JUMPER_ITF_STRING);
	QDBusMessage response = activityJumperItf->call("getPinState");
	if (response.type() != QDBusMessage::ErrorMessage) return response.arguments().at(0).toInt();
	else {
		qDebug() << "getPinState" << response.errorName();
		return 1;
	}
}

void ActivityJumperItf::desktopChanged() {
	//todo: load the pin status from Activity Jumper and store to a member, then implement the "refresh in qml" with onSignalDesktopChanged: { case ActivityJumper.currentPinState bla.source = ...;

	emit signalDesktopChanged();
}
