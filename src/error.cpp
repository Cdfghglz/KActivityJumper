//
// Created by cdfghglz on 22.5.2017.
//

#include <QDebug>
#include <QtCore/QStringList>

void invalidConfigEntry(QStringList entry, QString msg) {

	qDebug() << "Skipping configuration entry " << entry;
	qDebug() << msg;

}
