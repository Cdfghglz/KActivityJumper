
#include "plasmoidplugin.h"

#include <QtQml>
#include <kwindowsystem.h>

void PlasmoidPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.private.activityjumper"));

    qmlRegisterType<ActivityJumperItf>(uri, 1, 0, "ActivityJumper");
}

ActivityJumperItf::ActivityJumperItf(QObject *parent) : QObject(parent), m_consumer(new KActivities::Consumer) {
	connect(KWindowSystem::self(), &KWindowSystem::currentDesktopChanged, this, &ActivityJumperItf::desktopChanged);
	connect(m_consumer, &KActivities::Consumer::currentActivityChanged, this, &ActivityJumperItf::desktopChanged);

}
ActivityJumperItf::~ActivityJumperItf() { }

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
	if (response.type() != QDBusMessage::ErrorMessage) {
		return 0;
	}
	else {
		qDebug() << "changePinState: " << response.errorName();
		return 1;
	}
}

int ActivityJumperItf::getPinState() {
	QDBusInterface *activityJumperItf = initItfFromStringL(ACTIVITY_JUMPER_ITF_STRING);
	QDBusMessage response = activityJumperItf->call("getPinState");
	if (response.type() != QDBusMessage::ErrorMessage){

		PinStateEnum currentPinState = static_cast<PinStateEnum>(response.arguments().at(0).toInt());

		switch (currentPinState) {
			case pinState::UNPINNED :
				iconSource_ = "ajumper-unpin";
				break;
			case pinState::PINNED :
				iconSource_ = "ajumper-pin";
				break;
			case pinState::PINNED_LOCK :
				iconSource_ = "ajumper-pin-lock";
				break;
			case pinState::PINNED_KEY :
				iconSource_ = "ajumper-pin-key";
				break;
		}
		emit iconSourceChanged();
		return 0;
	}
	else {
		qDebug() << "getPinState: " << response.errorName();
		return 1;
	}
}

void ActivityJumperItf::desktopChanged() {
	getPinState();
	emit iconSourceChanged();
}
