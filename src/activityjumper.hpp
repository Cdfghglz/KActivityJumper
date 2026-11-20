#ifndef ACTIVITYJUMPER_HH
#define ACTIVITYJUMPER_HH

#include <QObject>
#include <QMap>
#include <QtDBus/QDBusInterface>
#include <QVector>

#include "common.h"

static const QStringList ACTIVITY_MAN_ITF_STRINGL = (QStringList()
		<< "org.kde.ActivityManager"
		<< "/ActivityManager/Activities"
		<< "org.kde.ActivityManager.Activities");
static const QStringList KWIN_ITF_STRINGL = (QStringList()
		<< "org.kde.KWin"
		<< "/KWin"
		<< "org.kde.KWin");

struct Position {
	static const uint itemsCt = 2;

	QString activityName;
	int desktopNr = 0;
	bool operator==(Position pos) {
		if (this->activityName == pos.activityName
				&& this->desktopNr == pos.desktopNr)
			return 1;
		return 0;
	}
};

class PinCtr {
private:
	QVector<bool> pinVec_;
	int activePtr_ = 0;

public:
	PinCtr(int maxSize);
	~PinCtr();

	int pinCt();
	int size();
	int registerNext();
	void free(QString str);

	void incrementActivePtr();
	int getCurrentPtr();
};

class ActivityJumper: public QObject
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityJumper");

private:
	QDBusInterface *initItfFromStringL(QStringList interfaceStringList);
	// Created once: building a QDBusInterface does a blocking Introspect call, and
	// the old per-call `new` leaked two of them on every position query/jump.
	QDBusInterface *activityManItf_ = nullptr;
	QDBusInterface *kwinItf_ = nullptr;
	static bool isValid(const Position &pos);
	static bool isDestinationKey(const QString &key);
	QMap<QString, QString> activityCodeMap_;
	QMap<QString, QString> activityNameMap_;
	void loadActivityMaps();
	QMap<QString, Position> destinationArgMap_;
	void loadDestinationMap();
	QStringList jumpHistory_;
	int historyLimit = 20; //todo: implement
	Position getCurrentPosition();
	void goToDestination(Position destination);
	void cleanupCurrentPin(pinState currentState, const QString &pinKey);
	void removeQuickPin(const QString &pinKey);

	int maxLockPins_ = 5;
	int maxQuickPins_ = 5;
	PinCtr lockPinCtr_;
	PinCtr quickPinCtr_;
	QString currentPinKey_;

	static pinState currentPinState_;
	pinState checkCurrentPinState();
	pinState checkCurrentPinState(Position currentPos);
	pinState checkPinState(Position pos, QString *pinKey);
	static QString positionString(const Position &pos);

public:
    ActivityJumper(QObject *parent = Q_NULLPTR);
    virtual ~ActivityJumper();

public slots:
    void jumpTo(QString destinArg);
	void jumpBack();
	void changePinState();
	void changePinState(pinState targetState);
	int getPinState();
	void onDesktopChanged();

signals:
	void pinStateChanged(int state);

};

#endif
