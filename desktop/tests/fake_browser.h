#pragma once

#include <QDesktopServices>
#include <QList>
#include <QObject>
#include <QUrl>

// Takes the place of the browser for https addresses while it exists, so a test sees what would have been opened.
class FakeBrowser : public QObject
{
    Q_OBJECT

public:
    FakeBrowser() { QDesktopServices::setUrlHandler(QStringLiteral("https"), this, "open"); }
    ~FakeBrowser() override { QDesktopServices::unsetUrlHandler(QStringLiteral("https")); }

    QList<QUrl> opened;

public slots:
    void open(const QUrl &url) { opened << url; }
};
