#include <QDebug>
#include <QDir>

#include "AJump.hpp"
#include <activityjumperadaptor.h>
#include "error.hpp"


ActivityJumper::ActivityJumper(QObject *parent)
		: QObject(parent), lockPinCtr_(maxLockPins_), quickPinCtr_(maxQuickPins_) {
	qDebug() << __PRETTY_FUNCTION__;
	new ActivityJumperAdaptor(this);
	QDBusConnection dbus = QDBusConnection::sessionBus();
	dbus.registerObject("/ActivityJumper", this);
	dbus.registerService("org.kde.ActivityJumper");

	loadActivityMaps();
	loadDestinationMap();
}

ActivityJumper::~ActivityJumper() {}

QDBusInterface *ActivityJumper::initItfFromStringL(QStringList interfaceStringList) {
	QDBusInterface *activityListInterface = new QDBusInterface(
			interfaceStringList[0], interfaceStringList[1], interfaceStringList[2],
			QDBusConnection::sessionBus(),
			this);
	return activityListInterface;
}

void ActivityJumper::loadActivityMaps() {

	QDBusInterface *activityManInterface = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	QDBusMessage activityListResponse = activityManInterface->call("ListActivities");

	if (activityListResponse.type() == QDBusMessage::ReplyMessage) {

		QListIterator<QString> itr(activityListResponse.arguments().at(0).toStringList());

		while (itr.hasNext()) {
			QString activityCode = itr.next();
			QString activityName = activityManInterface->call("ActivityName", activityCode).arguments().at(
					0).toString();
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
			} else {
				invalidConfigEntry(destinationEntry, "Invalid number of arguments.");
			}
		}
	} else {
		qDebug() << "Could not open the configuration file " << configFileName;
	}
}

Position ActivityJumper::getCurrentPosition() {

	QDBusInterface *activityManInterface = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	QDBusMessage activityListResponse = activityManInterface->call("CurrentActivity");

	QDBusInterface *kwinInterface = initItfFromStringL(KWIN_ITF_STRINGL);
	QDBusMessage kWinResponse = kwinInterface->call("currentDesktop");

	if (kWinResponse.type() == QDBusMessage::ReplyMessage &&
		activityListResponse.type() == QDBusMessage::ReplyMessage) {

		QString activityName = activityNameMap_[activityListResponse.arguments().at(0).toString()];
		int desktopNr = kWinResponse.arguments().at(0).toString().toInt();

		Position currentPos;
		currentPos.activityName = activityName;
		currentPos.desktopNr = desktopNr;
		return currentPos;

	} else if (kWinResponse.type() == QDBusMessage::ErrorMessage ||
			   activityListResponse.type() == QDBusMessage::ErrorMessage) {
		qDebug() << __PRETTY_FUNCTION__;
		qDebug() << activityListResponse.errorName();
		qDebug() << kWinResponse.errorName();
	}

	return Position();
}

void ActivityJumper::goToDestination(Position destination) {
	QDBusInterface *activityManInterface = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	QDBusInterface *kwinInterface = initItfFromStringL(KWIN_ITF_STRINGL);

	activityManInterface->call("SetCurrentActivity", activityCodeMap_[destination.activityName]);
	kwinInterface->call("setCurrentDesktop", destination.desktopNr);
}

void ActivityJumper::jumpTo(QString destinArg) {
	Position currentPos = getCurrentPosition();
	qDebug() << "jumping from " << jumpHistory_ << " to " << destinArg;
	if (!(currentPos == destinationArgMap_[destinArg])) {

		// repin if this is the first jump
		if (jumpHistory_.size() == 1 &&
				lockPinCtr_.nrPins() == 0 &&
				jumpHistory_.last().indexOf("pin") != -1) {
			QString quickToDel = jumpHistory_.takeLast();

			quickPinCtr_.free(quickToDel);
			QMap<QString, Position>::iterator it = destinationArgMap_.find(quickToDel);
			destinationArgMap_.erase(it);

			int pinNr = quickPinCtr_.nextFree();
			if (pinNr != -1) {
				QString historyStr = "quickpin" + QString::number(pinNr);
				destinationArgMap_[historyStr] = currentPos;
				jumpHistory_.append(historyStr);
				qDebug() << "repining and jumping from " << historyStr;
			}
		}

		if (jumpHistory_.isEmpty()) {
			changePinState();
			jumpHistory_.append(destinArg);
		} else if (jumpHistory_.last() != destinArg) {
			jumpHistory_.append(destinArg);
		}
		goToDestination(destinationArgMap_[destinArg]);
	}
}

void ActivityJumper::jumpBack() {

	if (!jumpHistory_.isEmpty()) {
		qDebug() << jumpHistory_;
		Position currentPos = getCurrentPosition();

		if (jumpHistory_.size() == lockPinCtr_.nrPins()) {
			// if only locked are left, cycle through them
			QString destKey = "lockpin" + QString::number(lockPinCtr_.getActive());
			Position prevPos = destinationArgMap_[destKey];

			qDebug() << "on " << currentPinKey_ << ", last to go to " << destKey;

			if (prevPos == currentPos) {
				lockPinCtr_.incrementActivePtr();
				destKey = "lockpin" + QString::number(lockPinCtr_.getActive());
				qDebug() << "incrementing to " << destKey;
			} else {
			}

			qDebug() << "we should be cirtling, first to " << destKey;
			qDebug() << destinationArgMap_.keys();
			goToDestination(destinationArgMap_[destKey]);
			lockPinCtr_.incrementActivePtr();

		} else {

			Position prevPos = destinationArgMap_[jumpHistory_.last()];

			if (prevPos == currentPos) {
				QString takenKey = jumpHistory_.takeLast();
				if (!jumpHistory_.isEmpty()) {
					prevPos = destinationArgMap_[jumpHistory_.last()];
				}
				if (takenKey.indexOf("quickpin") != -1) {
					QMap<QString, Position>::iterator it = destinationArgMap_.find(takenKey);
					destinationArgMap_.erase(it);
					quickPinCtr_.free(takenKey);
				}
				qDebug() << "jumping to " << takenKey;
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
				QString historyStr = "quickpin" + QString::number(pinNr);
				destinationArgMap_[historyStr] = currentPos;
				jumpHistory_.append(historyStr);
				qDebug() << "appending " << historyStr;
			}
			break;
		}

		case pinState::PINNED : {
			// change quickpin to lock pin, add it to home


			int pinNr = lockPinCtr_.nextFree();
			qDebug() << "checking if can be locked ";
			if (pinNr != -1) {
				QString historyStr = "lockpin" + QString::number(pinNr);
				qDebug() << "removing quick " << currentPinKey_ << ", adding " << historyStr;
				qDebug() << jumpHistory_;

				int pinHistoryPos = jumpHistory_.indexOf(currentPinKey_);
				if (pinHistoryPos != -1) jumpHistory_.removeAt(pinHistoryPos);

				QMap<QString, Position>::iterator it = destinationArgMap_.find(currentPinKey_);
				destinationArgMap_.erase(it);

				destinationArgMap_[historyStr] = currentPos;
				// add to home! increment lock count
				jumpHistory_.insert(lockPinCtr_.nrPins() - 1, historyStr);
				if (jumpHistory_.last() != historyStr) {
					jumpHistory_.append(historyStr);
				}

				quickPinCtr_.free(currentPinKey_);
				qDebug() << jumpHistory_;
			}
			break;
		}

		case pinState::PINNED_LOCK : {
			qDebug() << "unlocking " << currentPinKey_;
			while (jumpHistory_.indexOf(currentPinKey_) != -1) {
				int pinHistoryPos = jumpHistory_.indexOf(currentPinKey_);
				jumpHistory_.removeAt(pinHistoryPos);
				qDebug() << "cleaning from " << currentPinKey_ << " : " << jumpHistory_;
			}

			QMap<QString, Position>::iterator it = destinationArgMap_.find(currentPinKey_);
			destinationArgMap_.erase(it);

			lockPinCtr_.free(currentPinKey_);

			break;
		}

		case pinState::PINNED_KEY : {
			break;
		}

	}
//	qDebug() << "The pin status changed to " << currentPinState_ << ".";
}

int ActivityJumper::getPinState() {
	return static_cast<int>(checkCurrentPinState());
}

pinState ActivityJumper::checkCurrentPinState() {
	return checkCurrentPinState(getCurrentPosition());
}

pinState ActivityJumper::checkCurrentPinState(Position currentPos) {
	QList<Position> valuesList = destinationArgMap_.values();

	for (QMap<QString, Position>::iterator it = destinationArgMap_.begin(); it != destinationArgMap_.end(); ++it) {
		if (it.value() == currentPos) {
			currentPinKey_ = it.key();
			if (currentPinKey_.indexOf("lockpin") != -1) return pinState::PINNED_LOCK;
			else if (currentPinKey_.indexOf("quickpin") != -1) return pinState::PINNED;
			else return pinState::PINNED_KEY;
//			return pinState::PINNED_KEY; // todo: when returned to pinned only, unpin!
		}
	}
	return pinState::UNPINNED;
}
//
//void ActivityJumper::desktopChangedHandler() {
//	qDebug() << "ignlaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
//	if (jumpHistory_.size() == 1 &&
//		lockPinCtr_.nrPins() == 0 &&
//		jumpHistory_.last().indexOf("quickpin") != -1) {
//		QString quickToDel = jumpHistory_.takeLast();
//
//		quickPinCtr_.free(quickToDel);
//		QMap<QString, Position>::iterator it = destinationArgMap_.find(quickToDel);
//		destinationArgMap_.erase(it);
//		qDebug() << "freeing the previously taken quick --------";
//
//	}
//}

int PinCtr::nextFree() {
	qDebug() << "next pin requested when nrPins() = " << nrPins();
	if (pinVec.size() < nrPins()) return -1;
	for (int i = 0; i < pinVec.size(); ++i) {
		if (!pinVec[i]) {
			pinVec[i] = true;
//			nrPins_++;
			if (nrPins() == 1) activePtr_ = 0;
			qDebug() << "returning free pin nr. " << i;
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
	int nrToFree = str.split("n")[1].toInt();

	qDebug() << "freeing " << nrToFree << "from" << str;
	pinVec[nrToFree] = false;
//	nrPins_--;
	if (nrToFree == activePtr_) incrementActivePtr();
//	for (int i = 0; i < this->size(); ++i) {
//		qDebug() << pinVec[i];
//	}
//	qDebug() << nrPins();
}

void PinCtr::incrementActivePtr() {
	if (this->size() != 0) {
		for (int i = activePtr_ + 1; i < activePtr_ + 1 + this->size(); ++i) {
			int j;
			if (i >= this->size()) j = i - this->size();
			else j = i;
//			qDebug() << j;

			if (pinVec[j] == true) {
				activePtr_ = j;
				qDebug() << "setting active ptr to " << j;
				break;
			}
		}
	}
}

int PinCtr::getActive() {
	return activePtr_;
}

int PinCtr::nrPins() {
	int ctr = 0;
	for (int i = 0; i < this->size(); ++i) {
		if (pinVec[i] == true) {
			ctr++;
		}
	}
	return ctr;
}

