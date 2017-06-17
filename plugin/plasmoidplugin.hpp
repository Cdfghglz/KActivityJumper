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

#include <memory>
#include <QObject>
#include <QQmlExtensionPlugin>
#include <QtDBus/QDBusInterface>
#include <KActivities/Consumer>

#include "../src/common.h"

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

class ActivityJumperItf: public QObject {
	Q_OBJECT;
	Q_PROPERTY(QString iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged);
private:
	std::unique_ptr<KActivities::Consumer> consumer_;
	QDBusInterface *initItfFromStringL(QStringList interfaceStringList);
	QString iconSource_;

public:
	ActivityJumperItf(QObject *parent = nullptr);
	~ActivityJumperItf();

	QString iconSource() const {
		return iconSource_;
	}
	void setIconSource(const QString &sourceStr) {
		iconSource_ = sourceStr;
	}

signals:
	void iconSourceChanged();

public Q_SLOTS:
	int jumpBack();
	int changePinState();
	int getPinState();

	void desktopChanged();
};

#endif // PLASMOIDPLUGIN_H
