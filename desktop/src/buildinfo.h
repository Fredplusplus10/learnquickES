#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Facts about this build, shown on the start screen and useful in bug reports.
class BuildInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)
    Q_PROPERTY(QString qtVersion READ qtVersion CONSTANT)
    Q_PROPERTY(QString compiler READ compiler CONSTANT)
    Q_PROPERTY(QString platform READ platform CONSTANT)
    Q_PROPERTY(QString architecture READ architecture CONSTANT)
    Q_PROPERTY(QString latestReleaseUrl READ latestReleaseUrl CONSTANT)

public:
    using QObject::QObject;

    QString appVersion() const;
    QString qtVersion() const;
    QString compiler() const;
    QString platform() const;
    QString architecture() const;
    QString latestReleaseUrl() const;  // empty until lq/release.h names the public repository
};
