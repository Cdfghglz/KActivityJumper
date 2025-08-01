#include <QDebug>
#include <QDir>

#include "activityjumper.hpp"
#include <activityjumperadaptor.h>
#include "error.hpp"


ActivityJumper::ActivityJumper(QObject *parent)
		: QObject(parent), lockPinCtr_(maxLockPins_), quickPinCtr_(maxQuickPins_) {
	qDebug() << __PRETTY_FUNCTION__ << " initializing...";
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
			QString lineStr = fileStream.readLine();
			if (lineStr.indexOf("\"") != -1) {
				destinationEntry = lineStr.split("\"");
				destinationEntry.first() = destinationEntry.at(0).simplified().replace( " ", "" );
				destinationEntry.last() = destinationEntry.at(2).simplified().replace( " ", "" );
			} else destinationEntry = lineStr.split(" ");

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
	if (!(currentPos == destinationArgMap_[destinArg])) {
		
		// Auto-pin current position if it's unpinned before jumping away
		pinState currentState = checkCurrentPinState(currentPos);
		if (currentState == pinState::UNPINNED) {
			changePinState(pinState::PINNED);
		}

		// repin if this is the first jump
		if (jumpHistory_.size() == 1 &&
				lockPinCtr_.pinCt() == 0 &&
				jumpHistory_.last().indexOf("pin") != -1) {
			QString quickToDel = jumpHistory_.takeLast();

			quickPinCtr_.free(quickToDel);
			QMap<QString, Position>::iterator it = destinationArgMap_.find(quickToDel);
			destinationArgMap_.erase(it);

			int pinNr = quickPinCtr_.registerNext();
			if (pinNr != -1) {
				QString historyStr = "quickpin" + QString::number(pinNr);
				destinationArgMap_[historyStr] = currentPos;
				jumpHistory_.append(historyStr);
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
		Position currentPos = getCurrentPosition();

		if (jumpHistory_.size() == lockPinCtr_.pinCt()) {
			lockPinCtr_.incrementActivePtr();
			// if only locked are left, cycle through them
			QString destKey = "lockpin" + QString::number(lockPinCtr_.getCurrentPtr());
			Position prevPos = destinationArgMap_[destKey];

			if (prevPos == currentPos) {
				lockPinCtr_.incrementActivePtr();
				destKey = "lockpin" + QString::number(lockPinCtr_.getCurrentPtr());
			}

			goToDestination(destinationArgMap_[destKey]);

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
			}
			if (jumpHistory_.size() == lockPinCtr_.pinCt()) {
				QString destKey = "lockpin" + QString::number(lockPinCtr_.getCurrentPtr());
				prevPos = destinationArgMap_[destKey];
			}
			goToDestination(prevPos);
			
			// Unpin the destination we just jumped to, since we've "consumed" the pin
			Position newCurrentPos = getCurrentPosition();
			pinState newPinState = checkCurrentPinState(newCurrentPos);
			if (newPinState == pinState::PINNED) {
				changePinState(pinState::UNPINNED);
			}
		}
	}
}

pinState ActivityJumper::currentPinState_ = pinState::UNPINNED;

void ActivityJumper::changePinState(pinState targetState) {
	Position currentPos = getCurrentPosition();
	pinState currentState = checkCurrentPinState(currentPos);
	
	// If already in target state, do nothing
	if (currentState == targetState) {
		currentPinState_ = targetState;
		return;
	}
	
	// First, clean up current state if needed
	if (currentState != pinState::UNPINNED) {
		cleanupCurrentPin(currentState);
	}
	
	// Then set to target state
	switch (targetState) {
		case pinState::UNPINNED: {
			// Already cleaned up above, just update the state
			currentPinState_ = pinState::UNPINNED;
			break;
		}
		
		case pinState::PINNED: {
			int pinNr = quickPinCtr_.registerNext();
			if (pinNr != -1) {
				QString historyStr = "quickpin" + QString::number(pinNr);
				destinationArgMap_[historyStr] = currentPos;
				jumpHistory_.append(historyStr);
				currentPinState_ = pinState::PINNED;
			}
			break;
		}
		
		case pinState::PINNED_LOCK: {
			int pinNr = lockPinCtr_.registerNext();
			if (pinNr != -1) {
				QString historyStr = "lockpin" + QString::number(pinNr);
				destinationArgMap_[historyStr] = currentPos;
				// Add to locks history! increment lock count
				jumpHistory_.insert(lockPinCtr_.pinCt() - 1, historyStr);
				if (jumpHistory_.last() != historyStr) {
					jumpHistory_.append(historyStr);
				}
				currentPinState_ = pinState::PINNED_LOCK;
			}
			break;
		}
		
		case pinState::PINNED_KEY: {
			// PINNED_KEY state is for predefined destination keys, 
			// cannot be set programmatically
			currentPinState_ = currentState; // Keep current state
			break;
		}
	}
}

void ActivityJumper::cleanupCurrentPin(pinState currentState) {
	switch (currentState) {
		case pinState::PINNED: {
			// Remove quickpin
			int pinHistoryPos = jumpHistory_.indexOf(currentPinKey_);
			if (pinHistoryPos != -1) jumpHistory_.removeAt(pinHistoryPos);
			
			QMap<QString, Position>::iterator it = destinationArgMap_.find(currentPinKey_);
			if (it != destinationArgMap_.end()) {
				destinationArgMap_.erase(it);
			}
			
			quickPinCtr_.free(currentPinKey_);
			break;
		}
		
		case pinState::PINNED_LOCK: {
			// Remove lockpin
			while (jumpHistory_.indexOf(currentPinKey_) != -1) {
				int pinHistoryPos = jumpHistory_.indexOf(currentPinKey_);
				jumpHistory_.removeAt(pinHistoryPos);
			}
			
			QMap<QString, Position>::iterator it = destinationArgMap_.find(currentPinKey_);
			if (it != destinationArgMap_.end()) {
				destinationArgMap_.erase(it);
			}
			
			lockPinCtr_.free(currentPinKey_);
			break;
		}
		
		case pinState::PINNED_KEY: {
			// PINNED_KEY entries are permanent, don't remove them
			break;
		}
		
		case pinState::UNPINNED: {
			// Nothing to clean up
			break;
		}
	}
}

void ActivityJumper::changePinState() {
	// Pinning state machine cycling through UNPINNED, PINNED, PINNED_LOCK
	Position currentPos = getCurrentPosition();
	currentPinState_ = checkCurrentPinState(currentPos);

	switch (currentPinState_) {
		case pinState::UNPINNED : {
			changePinState(pinState::PINNED);
			break;
		}

		case pinState::PINNED : {
			changePinState(pinState::PINNED_LOCK);
			break;
		}

		case pinState::PINNED_LOCK : {
			changePinState(pinState::UNPINNED);
			break;
		}

		case pinState::PINNED_KEY : {
			// do nothing
			break;
		}
	}
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
		}
	}
	return pinState::UNPINNED;
}

int PinCtr::registerNext() {
	if (pinVec_.size() < pinCt()) return -1;
	for (int i = 0; i < pinVec_.size(); ++i) {
		if (!pinVec_[i]) {
			pinVec_[i] = true;
			if (pinCt() == 1) activePtr_ = 0;
			return i;
		}
	}
	return -1;
}

PinCtr::PinCtr(int maxSize) : pinVec_(maxSize) { }

PinCtr::~PinCtr() { }

int PinCtr::size() {
	return pinVec_.size();
}

void PinCtr::free(QString str) {
	int nrToFree = str.split("n")[1].toInt();
	pinVec_[nrToFree] = false;

	if (nrToFree == activePtr_) incrementActivePtr();
}

void PinCtr::incrementActivePtr() {
	if (this->size() != 0) {
		for (int i = activePtr_ + 1; i < activePtr_ + 1 + this->size(); ++i) {
			int j;
			if (i >= this->size()) j = i - this->size();
			else j = i;

			if (pinVec_[j] == true) {
				activePtr_ = j;
				break;
			}
		}
	}
}

int PinCtr::getCurrentPtr() {
	return activePtr_;
}

int PinCtr::pinCt() {
	int ctr = 0;
	for (int i = 0; i < this->size(); ++i) {
		if (pinVec_[i] == true) {
			ctr++;
		}
	}
	return ctr;
}

