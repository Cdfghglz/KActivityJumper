#include <QDebug>

#include "AJump.hpp"
#include <activityjumperadaptor.h>

ActivityJumper::ActivityJumper(QObject *parent)
    : QObject(parent)
{
    qDebug() << __PRETTY_FUNCTION__;
    new ActivityJumperAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerObject("/org/kde/ActivityJumper", this);
    dbus.registerService("org.kde.ActivityJumper");
}

ActivityJumper::~ActivityJumper() {}

void ActivityJumper::jumpTo(QString destinArg)
{
    qDebug() << __PRETTY_FUNCTION__ << "recieved" << destinArg;
}

void ActivityJumper::jumpBack()
{
    qDebug() << __PRETTY_FUNCTION__;
}
