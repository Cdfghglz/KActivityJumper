//
// Created by cdfghglz on 4.6.2017.
//

#include <QObject>
#include "AJump.hpp"

class JumperModel : public QObject {
	Q_OBJECT;
private:
	ActivityJumper aJumper;
//	Q_PROPERTY()
//	Q_PROPERTY(void jumpBack READ jumpBack NOTIFY )
//	Q_PRIVATE_SLOT()
public:
	Q_INVOKABLE void jumpBackInvok {
		aJumper.jumpBack();
	};
};

class JumperPluginInterface: public QQmlExtensionPlugin
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
	void registerTypes(const char *uri)
	{
		Q_ASSERT(uri == QLatin1String("ActivityJumperPlug"));
		qmlRegisterType<ActivityJumper>(uri, 1, 0, "ActivityJumper");
	}
};

#include "plugin.moc"
