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

#ifndef PLASMOIDPLUGIN_H
#define PLASMOIDPLUGIN_H

#include <QObject>
#include <QQmlExtensionPlugin>
#include <QtDBus/QDBusInterface>

static const QStringList ACTIVITY_JUMPER_ITF_STRING = (QStringList()
		<< "org.kde.ActivityJumper"
		<< "/ActivityJumper"
		<< "org.kde.ActivityJumper");

class QQmlEngine;
class PlasmoidPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

class DBusInterface : public QObject {
	Q_OBJECT
private:
	QDBusInterface *initItfFromStringL(QStringList interfaceStringList);

public Q_SLOTS:
	int jumpBack();
	int changePinStatus();
};

#endif // PLASMOIDPLUGIN_H
