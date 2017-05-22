#include <QDebug>

#include "AJump.hpp"
#include <activityjumperadaptor.h>
#include "error.hpp"

ActivityJumper::ActivityJumper(QObject *parent)
		: QObject(parent) {
	qDebug() << __PRETTY_FUNCTION__;
	new ActivityJumperAdaptor(this);
	QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerObject("/ActivityJumper", this);
	dbus.registerService("org.kde.ActivityJumper");

	loadActivityCodeMap();
	loadDestinationMap();
}

ActivityJumper::~ActivityJumper() {}

void ActivityJumper::loadActivityCodeMap() {
	QDBusInterface *activityListInterface = new QDBusInterface("org.kde.ActivityManager",
															   "/ActivityManager/Activities",
															   "org.kde.ActivityManager.Activities",
															   QDBusConnection::sessionBus(),
															   this);

	QDBusMessage activityListResponse = activityListInterface->call("ListActivities");

	if (activityListResponse.type() == QDBusMessage::ReplyMessage) {

		QListIterator<QString> itr(activityListResponse.arguments().at(0).toStringList());

		while (itr.hasNext()) {
			QString activityCode = itr.next();
			QString activityName = activityListInterface->call("ActivityName", activityCode).arguments().at(
					0).toString();
			activityCodeMap_.insert(activityName, activityCode);
		}

	} else if (activityListResponse.type() == QDBusMessage::ErrorMessage) {
		qDebug() << __PRETTY_FUNCTION__;
		qDebug() << activityListResponse.errorName();
	}

}

void ActivityJumper::loadDestinationMap() {

	QString configFileName = "../jumperDestinationConfig.txt";
	QFile configFile(configFileName);

	if (configFile.open(QIODevice::ReadOnly)) {
		QTextStream fileStream(&configFile);
		QStringList destinationEntry;

		while (!fileStream.atEnd()) {
			destinationEntry = fileStream.readLine().split(" ");

			if (destinationEntry.count() == (1 + Destination::itemsCt)) {

				QString destinationArgument = destinationEntry.at(0);
				if (destinationMap_.uniqueKeys().contains(destinationArgument)) {
					invalidConfigEntry(destinationEntry, "Destination argument not unique.");
					continue;
				}

				Destination destination;
				if (!activityCodeMap_.keys().contains(destinationEntry.at(1))) {
					invalidConfigEntry(destinationEntry, "Activity name does not exist.");
					continue;
				}
				destination.activity = destinationEntry.at(1);

				QRegExp re("\\d*");
				if (!re.exactMatch(destinationEntry.at(2))) {
					invalidConfigEntry(destinationEntry, "Destination desktop not a digit.");
					continue;
				}
				destination.desktop = destinationEntry.at(2).toInt();

				destinationMap_.insert(destinationArgument, destination);
			}
			else {
				invalidConfigEntry(destinationEntry, "Invalid number of arguments.");
			}
		}
	}
	else {
		qDebug() << "Could not open the configuration file.";
	}
}

void ActivityJumper::jumpTo(QString destinArg) {
	qDebug() << __PRETTY_FUNCTION__ << "recieved" << destinArg;
}

void ActivityJumper::jumpBack() {
	qDebug() << __PRETTY_FUNCTION__;
}
