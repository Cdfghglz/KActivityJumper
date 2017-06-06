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

	QString activityName;
	int desktopNr;
	bool operator==(Position pos) {
		if (this->activityName == pos.activityName
				&& this->desktopNr == pos.desktopNr)
			return 1;
		return 0;
	}
};

class ActivityJumper: public QObject
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityJumper");

private:
	QDBusInterface *initItfFromStringL(QStringList interfaceStringList);
	QMap<QString, QString> activityCodeMap_;
	QMap<QString, QString> activityNameMap_;
	void loadActivityMaps();
	QMap<QString, Position> destinationArgMap_;
	void loadDestinationMap();
	QStringList jumpHistory_;
	Position getCurrentPosition();
	void goToDestination(Position destination);

public:
    ActivityJumper(QObject *parent = Q_NULLPTR);
    virtual ~ActivityJumper();

public slots:
    void jumpTo(QString destinArg);
	void jumpBack();

};

#endif
