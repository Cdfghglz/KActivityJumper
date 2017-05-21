#include <QDebug>

#include "AJump.hpp"
#include <activityjumperadaptor.h>

ActivityJumper::ActivityJumper(QObject *parent)
		: QObject(parent) {
	qDebug() << __PRETTY_FUNCTION__;
	new ActivityJumperAdaptor(this);
	QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerObject("/ActivityJumper", this);
	dbus.registerService("org.kde.ActivityJumper");
	loadActivityCodeMap();
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
			QString activityName = activityListInterface->call("ActivityName", activityCode).arguments().at(0).toString();
			activityCodeMap_.insert(activityName, activityCode);
		}

	}
	else if (activityListResponse.type() == QDBusMessage::ErrorMessage) {
		qDebug() << __PRETTY_FUNCTION__;
		qDebug() << activityListResponse.errorName();
	}

}

void ActivityJumper::jumpTo(QString destinArg) {
	qDebug() << __PRETTY_FUNCTION__ << "recieved" << destinArg;
}

void ActivityJumper::jumpBack() {
	qDebug() << __PRETTY_FUNCTION__;
}
