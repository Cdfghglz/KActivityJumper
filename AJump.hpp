#ifndef ACTIVITYJUMPER_HH
#define ACTIVITYJUMPER_HH

#include <QObject>
#include <QMap>

struct Destination {
	static const uint itemsCt = 2;

	QString activity;
	int desktop;
};

class ActivityJumper: public QObject
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityJumper");

private:
	QMap<QString, QString> activityCodeMap_;
	void loadActivityCodeMap();
	QMap<QString, Destination> destinationMap_;
	void loadDestinationMap();

public:
    ActivityJumper(QObject *parent);
    virtual ~ActivityJumper();

public slots:
    void jumpTo(QString destinArg);
	void jumpBack();

};

#endif
