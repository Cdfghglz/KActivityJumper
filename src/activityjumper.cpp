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

	activityManItf_ = initItfFromStringL(ACTIVITY_MAN_ITF_STRINGL);
	kwinItf_ = initItfFromStringL(KWIN_ITF_STRINGL);

	loadActivityMaps();
	loadDestinationMap();

	// Tell the panel widget the pin state of wherever we are now, on every desktop
	// and activity change. (The 2026 build used KWindowSystem, which does not work
	// in a QCoreApplication -- "Cannot use KWindowSystem without a QGuiApplication"
	// -- so the icon only updated on activity switches and went stale.)
	dbus.connect("org.kde.KWin", "/VirtualDesktopManager", "org.kde.KWin.VirtualDesktopManager",
				 "currentChanged", this, SLOT(onDesktopChanged()));
	dbus.connect(ACTIVITY_MAN_ITF_STRINGL[0], ACTIVITY_MAN_ITF_STRINGL[1], ACTIVITY_MAN_ITF_STRINGL[2],
				 "CurrentActivityChanged", this, SLOT(onDesktopChanged()));
}

bool ActivityJumper::isValid(const Position &pos) {
	return !pos.activityName.isEmpty() && pos.desktopNr > 0;
}

QString ActivityJumper::positionString(const Position &pos) {
	return pos.activityName + ":" + QString::number(pos.desktopNr);
}

void ActivityJumper::onDesktopChanged() {
	emit pinStateChanged(static_cast<int>(checkCurrentPinState()));
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

	QDBusMessage activityListResponse = activityManItf_->call("CurrentActivity");
	QDBusMessage kWinResponse = kwinItf_->call("currentDesktop");

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
	if (!isValid(destination)) {
		qDebug() << "goToDestination: invalid destination" << positionString(destination) << "- not jumping";
		return;
	}
	activityManItf_->call("SetCurrentActivity", activityCodeMap_[destination.activityName]);
	kwinItf_->call("setCurrentDesktop", destination.desktopNr);
}

void ActivityJumper::jumpTo(QString destinArg) {
	Position currentPos = getCurrentPosition();
	if (!isValid(currentPos) || !destinationArgMap_.contains(destinArg)) {
		qDebug() << "jumpTo" << destinArg << ": invalid position or unknown destination, ignoring";
		return;
	}
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
			if (quickToDel.indexOf("quickpin") != -1) {
				removeQuickPin(quickToDel);
			}

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
		emit pinStateChanged(static_cast<int>(checkCurrentPinState()));
	}
}

void ActivityJumper::jumpBack() {

	if (!jumpHistory_.isEmpty()) {
		Position currentPos = getCurrentPosition();
		if (!isValid(currentPos)) {
			qDebug() << "jumpBack: could not read current position, ignoring";
			return;
		}

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
			QString destKey = jumpHistory_.last();
			Position prevPos = destinationArgMap_[destKey];

			if (prevPos == currentPos) {
				QString takenKey = jumpHistory_.takeLast();
				destKey.clear();
				if (!jumpHistory_.isEmpty()) {
					destKey = jumpHistory_.last();
					prevPos = destinationArgMap_[destKey];
				}
				if (takenKey.indexOf("quickpin") != -1) {
					removeQuickPin(takenKey);
				}
			}
			if (jumpHistory_.size() == lockPinCtr_.pinCt() && lockPinCtr_.pinCt() > 0) {
				destKey = "lockpin" + QString::number(lockPinCtr_.getCurrentPtr());
				prevPos = destinationArgMap_[destKey];
			}
			goToDestination(prevPos);

			// Consume the quick pin we just jumped to. Decided by the key we jumped
			// to, NOT by re-reading the position: the old code re-read it right after
			// the jump (could still be the old spot) and then changePinState(UNPINNED)
			// read it a second time -- if that read had landed on a lock, the lock
			// was removed. Locks and destination keys are never consumed here.
			if (destKey.indexOf("quickpin") != -1) {
				removeQuickPin(destKey);
			}
		}
		emit pinStateChanged(static_cast<int>(checkCurrentPinState()));
	}
}

pinState ActivityJumper::currentPinState_ = pinState::UNPINNED;

void ActivityJumper::changePinState(pinState targetState) {
	Position currentPos = getCurrentPosition();
	if (!isValid(currentPos)) {
		qDebug() << "changePinState: could not read current position, ignoring";
		return;
	}
	QString pinKey;
	pinState currentState = checkPinState(currentPos, &pinKey);
	
	// If already in target state, do nothing
	if (currentState == targetState) {
		currentPinState_ = targetState;
		return;
	}
	
	// First, clean up current state if needed (the pin found at THIS position)
	if (currentState != pinState::UNPINNED) {
		cleanupCurrentPin(currentState, pinKey);
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
				qDebug() << "pin: added" << historyStr << "at" << positionString(currentPos);
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
				qDebug() << "pin: added" << historyStr << "at" << positionString(currentPos);
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
	emit pinStateChanged(static_cast<int>(checkPinState(currentPos, nullptr)));
}

void ActivityJumper::removeQuickPin(const QString &pinKey) {
	if (pinKey.indexOf("quickpin") == -1) {
		return;
	}
	jumpHistory_.removeAll(pinKey);
	QMap<QString, Position>::iterator it = destinationArgMap_.find(pinKey);
	if (it != destinationArgMap_.end()) {
		qDebug() << "pin: consumed" << pinKey << "at" << positionString(it.value());
		destinationArgMap_.erase(it);
		quickPinCtr_.free(pinKey);
	}
}

void ActivityJumper::cleanupCurrentPin(pinState currentState, const QString &pinKey) {
	const QString currentPinKey_ = pinKey; // explicit key, not the shared member
	if (destinationArgMap_.contains(currentPinKey_)) {
		qDebug() << "pin: removed" << currentPinKey_ << "at" << positionString(destinationArgMap_[currentPinKey_]);
	}
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
	return checkPinState(currentPos, &currentPinKey_);
}

pinState ActivityJumper::checkPinState(Position pos, QString *pinKey) {
	// A position can carry several entries (e.g. a destination key and a lock).
	// Report the strongest: lock > quick pin > destination key.
	QString lockKey, quickKey, destKey;
	for (QMap<QString, Position>::iterator it = destinationArgMap_.begin(); it != destinationArgMap_.end(); ++it) {
		if (it.value() == pos) {
			if (it.key().indexOf("lockpin") != -1) { if (lockKey.isEmpty()) lockKey = it.key(); }
			else if (it.key().indexOf("quickpin") != -1) { if (quickKey.isEmpty()) quickKey = it.key(); }
			else if (destKey.isEmpty()) destKey = it.key();
		}
	}
	pinState state = pinState::UNPINNED;
	QString key;
	if (!lockKey.isEmpty()) { state = pinState::PINNED_LOCK; key = lockKey; }
	else if (!quickKey.isEmpty()) { state = pinState::PINNED; key = quickKey; }
	else if (!destKey.isEmpty()) { state = pinState::PINNED_KEY; key = destKey; }
	if (pinKey) *pinKey = key;
	return state;
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

