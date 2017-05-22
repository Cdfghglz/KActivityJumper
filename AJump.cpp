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

	loadActivityMaps();
	loadDestinationMap();
}

ActivityJumper::~ActivityJumper() {}

QDBusInterface * ActivityJumper::initItfFromStringL(QStringList interfaceStringList) {
	QDBusInterface *activityListInterface = new QDBusInterface(
			interfaceStringList[0], interfaceStringList[1], interfaceStringList[2],
			QDBusConnection::sessionBus(),
			this);
	return activityListInterface;
}

void ActivityJumper::loadActivityMaps() {

	QDBusInterface* activityManInterface = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	QDBusMessage activityListResponse = activityManInterface->call("ListActivities");

	if (activityListResponse.type() == QDBusMessage::ReplyMessage) {

		QListIterator<QString> itr(activityListResponse.arguments().at(0).toStringList());

		while (itr.hasNext()) {
			QString activityCode = itr.next();
			QString activityName = activityManInterface->call("ActivityName", activityCode).arguments().at(0).toString();
			activityCodeMap_.insert(activityName, activityCode);
			activityNameMap_.insert(activityCode, activityName);
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

			if (destinationEntry.count() == (1 + Position::itemsCt)) {

				QString destinationArgument = destinationEntry.at(0);
				if (destinationArgMap_.uniqueKeys().contains(destinationArgument)) {
					invalidConfigEntry(destinationEntry, "Destination argument not unique.");
					continue;
				}

				Position destination;
				if (!activityCodeMap_.keys().contains(destinationEntry.at(1))) {
					invalidConfigEntry(destinationEntry, "Activity name does not exist.");
					continue;
				}
				destination.activityName = destinationEntry.at(1);

				QRegExp re("\\d*");
				if (!re.exactMatch(destinationEntry.at(2))) {
					invalidConfigEntry(destinationEntry, "Destination desktop not a digit.");
					continue;
				}
				destination.desktopNr = destinationEntry.at(2).toInt();

				destinationArgMap_.insert(destinationArgument, destination);
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

Position ActivityJumper::getCurrentPosition() {

	QDBusInterface* activityManInterface = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	QDBusMessage activityListResponse = activityManInterface->call("CurrentActivity");

	QDBusInterface* kwinInterface = initItfFromStringL(KWIN_ITF_STRINGL);
	QDBusMessage kWinResponse = kwinInterface->call("currentDesktop");

	if (kWinResponse.type() == QDBusMessage::ReplyMessage && activityListResponse.type() == QDBusMessage::ReplyMessage) {

		QString activityName = activityNameMap_[activityListResponse.arguments().at(0).toString()];
		int desktopNr = kWinResponse.arguments().at(0).toString().toInt();

		Position currentPos;
		currentPos.activityName = activityName;
		currentPos.desktopNr = desktopNr;
		return currentPos;

	} else if (kWinResponse.type() == QDBusMessage::ErrorMessage || activityListResponse.type() == QDBusMessage::ErrorMessage) {
		qDebug() << __PRETTY_FUNCTION__;
		qDebug() << activityListResponse.errorName();
		qDebug() << kWinResponse.errorName();
	}

	return Position();
}

void ActivityJumper::goToDestination(Position destination) {
	QDBusInterface* activityManInterface = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	QDBusInterface* kwinInterface = initItfFromStringL(KWIN_ITF_STRINGL);

	activityManInterface->call("SetCurrentActivity", activityCodeMap_[destination.activityName]);
	kwinInterface->call("setCurrentDesktop", destination.desktopNr);
}

void ActivityJumper::jumpTo(QString destinArg) {
	Position currentPos = getCurrentPosition();
	if (!(currentPos == destinationArgMap_[destinArg])) {
		if (jumpHistory_.empty()) {
			initialPosition_ = currentPos;
		}

		goToDestination(destinationArgMap_[destinArg]);
		jumpHistory_.append(destinArg);
	}
}

void ActivityJumper::jumpBack() {
	Position currentPos = getCurrentPosition();

	Position prevPos;
	if (jumpHistory_.isEmpty()) prevPos = initialPosition_;
	else prevPos = destinationArgMap_[jumpHistory_.takeLast()];

	if (!(currentPos == prevPos)) goToDestination(prevPos);
}
