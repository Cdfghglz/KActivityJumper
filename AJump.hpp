#ifndef ACTIVITYJUMPER_HH
#define ACTIVITYJUMPER_HH

#include <QObject>
#include <QMap>
#include <QtDBus/QDBusInterface>

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

	QString activity;
	int desktop;
};

class ActivityJumper: public QObject
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityJumper");

private:
//	QDBusInterface* activityManagerInterface_;
	QDBusInterface *initItfFromStringL(QStringList interfaceStringList);
	QMap<QString, QString> activityCodeMap_;
	QMap<QString, QString> activityNameMap_;
	void loadActivityMaps();
	QMap<QString, Position> destinationMap_;
	void loadDestinationMap();
	QList<QString> jumpHistory_;
	Position initialPosition;
	Position getCurrentPosition();
	void goToDestination(Position destination);

public:
    ActivityJumper(QObject *parent);
    virtual ~ActivityJumper();

public slots:
    void jumpTo(QString destinArg);
	void jumpBack();

};

#endif
