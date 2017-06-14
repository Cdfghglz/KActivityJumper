#include <QDebug>
#include <QDir>

#include "AJump.hpp"
#include <activityjumperadaptor.h>
#include "error.hpp"

ActivityJumper::ActivityJumper(QObject *parent)
		: QObject(parent), lockPinCtr_(maxLockPins_), quickPinCtr_(maxQuickPins_){
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

	QString configFileName = QDir::homePath() +
			"/.ActivityJumper/jumperDestinationConfig.config";
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
		qDebug() << "Could not open the configuration file " << configFileName;
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

		if (jumpHistory_.isEmpty()) {
			changePinState();
			jumpHistory_.append(destinArg);
		}
		else if (jumpHistory_.last() != destinArg) {
			jumpHistory_.append(destinArg);
		}
		goToDestination(destinationArgMap_[destinArg]);

	}
}

void ActivityJumper::jumpBack() {

	if (!jumpHistory_.isEmpty()) {
		qDebug() << jumpHistory_;
		Position currentPos = getCurrentPosition();

		if (jumpHistory_.size() == lockPinCtr_.nrPins_) {
			qDebug() << "mozne?";
			// cycle through lock pinned
			QString destKey = "lockpin" + QString::number(lockPinCtr_.getActive());
			Position prevPos = destinationArgMap_[destKey];

			if (prevPos == currentPos) {
				incrementLockPtr();
				QString destKey = "lockpin" + QString::number(lockPinCtr_.getActive());
			}
			goToDestination(destinationArgMap_[destKey]);

		}
		else {

			Position prevPos = destinationArgMap_[jumpHistory_.last()];

			if (prevPos == currentPos) {
				QString takenKey = jumpHistory_.takeLast();
				if (!jumpHistory_.isEmpty()) {
					prevPos = destinationArgMap_[jumpHistory_.last()];
				}
				if (takenKey.indexOf("pin") != -1) {
					QMap<QString, Position>::iterator it = destinationArgMap_.find(takenKey);
					destinationArgMap_.erase(it);
					quickPinCtr_.free(takenKey);
				}
				qDebug() << "going back to " << takenKey;
			}
			goToDestination(prevPos);
		}
	}
}

pinState ActivityJumper::currentPinState_ = pinState::UNPINNED;

void ActivityJumper::changePinState() {
	//todo: implement the logic

	Position currentPos = getCurrentPosition();
	currentPinState_ = checkCurrentPinState(currentPos);

	switch (currentPinState_) {
		case pinState::UNPINNED : {
			int pinNr = quickPinCtr_.nextFree();
			if (pinNr != -1) {
				QString historyStr = "pin" + QString::number(pinNr);
				destinationArgMap_[historyStr] = currentPos;
				jumpHistory_.append(historyStr);
				qDebug() << historyStr;
			}
			break;
		}

		case pinState::PINNED : {
			// change quickpin to lock pin, add it to home
			break;
		}

		case pinState::PINNED_LOCK : {
			break;
		}

		case pinState::PINNED_KEY : {
			break;
		}

	}
	qDebug() << "The pin status changed to " << currentPinState_ << ".";
}

int ActivityJumper::getPinState() {
	return static_cast<int>(checkCurrentPinState());
}

pinState ActivityJumper::checkCurrentPinState() {
	return checkCurrentPinState(getCurrentPosition());
}

pinState ActivityJumper::checkCurrentPinState(Position currentPos) {
	QList<Position> valuesList = destinationArgMap_.values();

	for (QMap<QString, Position>::iterator it = destinationArgMap_.begin(); it != destinationArgMap_.end(); ++it)
	{
		if (it.value() == currentPos)
		{
			currentPinKey_ = it.key();
			if (currentPinKey_.indexOf("pin") != -1) {
				int firstIdx = jumpHistory_.indexOf(currentPinKey_);
				if (firstIdx >= 0 && firstIdx >= lockPinCtr_.nrPins_) return pinState::PINNED;
				return pinState::PINNED_LOCK;
			}
			return pinState::PINNED_KEY;
		}
	}
	return pinState::UNPINNED;
}

void ActivityJumper::incrementLockPtr() {
	while (currentLockPtr_ > lockPinCtr_.size()) currentLockPtr_--;
	currentLockPtr_++;
	if (currentLockPtr_ > lockPinCtr_.size()) {
		currentLockPtr_ = 0;
	}
}

int PinCtr::nextFree() {
	if (pinVec.size() <= nrPins_) return -1;
	for (int i = 0; i < pinVec.size(); ++i) {
		if (!pinVec[i]) {
			pinVec[i] = true;
			nrPins_++;
			return i;
		}
	}
	return -1;
}

PinCtr::PinCtr(int maxSize) : pinVec(maxSize) {

}

PinCtr::~PinCtr() {

}

int PinCtr::size() {
	return pinVec.size();
}

void PinCtr::free(QString str) {
	int nrToFree = str.toInt();
	pinVec[nrToFree] = false;
	nrPins_--;
	if (nrToFree == activePtr_) incrementActivePtr();
}

void PinCtr::incrementActivePtr() {
	if (this->size() != 0) {
		for (int i = activePtr_; i < activePtr_ + this->size(); ++i) {
			if (i >= this->size()) {
				i = i - this->size();
			}
			if (pinVec[i] == true) {
				activePtr_ = i;
			}
		}
	}
}

int PinCtr::getActive() {
	return activePtr_;
}


