#ifndef ACTIVITYJUMPER_HH
#define ACTIVITYJUMPER_HH

#include <QObject>
#include <QMap>

class ActivityJumper: public QObject
{
    Q_OBJECT;
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityJumper");

private:
QMap<QString, QString> activityCodeMap_;
	void loadActivityCodeMap();
public:
    ActivityJumper(QObject *parent);
    virtual ~ActivityJumper();

public slots:
    void jumpTo(QString destinArg);
	void jumpBack();

};

#endif
