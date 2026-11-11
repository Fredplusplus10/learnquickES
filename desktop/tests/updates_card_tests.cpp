// The Settings card that sends the learner to the downloads, clicked the way a learner clicks it. A stand-in catches
// the address, so no browser opens and the test goes nowhere near the network.
#include "fake_browser.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <QtQuickTest/quicktest.h>

#include <functional>

namespace {

constexpr char kScene[] = R"(
import QtQuick

Rectangle {
    width: 700
    height: 260

    UpdatesCard {
        objectName: "card"
        x: 20
        y: 20
        width: 660
        version: "1.2.3"
        releaseUrl: "https://github.com/octo-cat/example/releases/latest"
    }
}
)";

// Theme follows the Settings screen, a C++ singleton in the app; here a stand-in in day mode.
constexpr char kPreferences[] = R"(
pragma Singleton
import QtQuick

QtObject {
    property string theme: "light"
}
)";

// The card and what it uses, outside the app's QML module: the source files next to a qmldir.
bool writeScene(const QTemporaryDir &dir)
{
    for (const char *name : {"UpdatesCard.qml", "Card.qml", "PrimaryButton.qml", "Icon.qml", "Theme.qml"}) {
        if (!QFile::copy(QStringLiteral(LQ_QML_DIR "/") + QLatin1String(name), dir.filePath(QLatin1String(name))))
            return false;
    }
    QFile qmldir(dir.filePath(QStringLiteral("qmldir")));
    QFile preferences(dir.filePath(QStringLiteral("Preferences.qml")));
    QFile scene(dir.filePath(QStringLiteral("Scene.qml")));
    return qmldir.open(QIODevice::WriteOnly)
           && qmldir.write("singleton Theme 1.0 Theme.qml\nsingleton Preferences 1.0 Preferences.qml\n") > 0
           && preferences.open(QIODevice::WriteOnly) && preferences.write(kPreferences) > 0
           && scene.open(QIODevice::WriteOnly) && scene.write(kScene) > 0;
}

struct Scene {
    QStringList warnings;  // before the view, which can still warn while it is destroyed
    QQuickView view;
    QQuickItem *card = nullptr;
    QQuickItem *button = nullptr;

    void click()
    {
        REQUIRE(QQuickTest::qWaitForPolish(&view));
        const QPoint centre = button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
        QTest::mouseClick(&view, Qt::LeftButton, {}, centre);
    }
};

void withScene(const std::function<void(Scene &, FakeBrowser &)> &test)
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    REQUIRE(writeScene(dir));
    FakeBrowser browser;
    Scene scene;
    QObject::connect(scene.view.engine(), &QQmlEngine::warnings, [&scene](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings)
            scene.warnings << warning.toString();
    });
    scene.view.setSource(QUrl::fromLocalFile(dir.filePath(QStringLiteral("Scene.qml"))));
    INFO(scene.warnings.join(QLatin1Char('\n')).toStdString());
    REQUIRE(scene.view.status() == QQuickView::Ready);
    scene.card = scene.view.rootObject()->findChild<QQuickItem *>(QStringLiteral("card"));
    REQUIRE(scene.card != nullptr);
    scene.button = scene.card->findChild<QQuickItem *>(QStringLiteral("downloadLatest"));
    REQUIRE(scene.button != nullptr);
    scene.view.show();
    REQUIRE(QTest::qWaitForWindowExposed(&scene.view));
    test(scene, browser);
    INFO(scene.warnings.join(QLatin1Char('\n')).toStdString());
    CHECK(scene.warnings.isEmpty());
}

} // namespace

TEST_CASE("the card opens nothing by itself, and one click opens the latest release once", "[updates]")
{
    withScene([](Scene &s, FakeBrowser &browser) {
        QCoreApplication::processEvents();
        CHECK(browser.opened.isEmpty());
        // The version the learner has, and how often to look for a newer one: the app cannot look by itself.
        CHECK(s.card->property("subtitle").toString()
              == QStringLiteral("Version 1.2.3 · Check every 15 days for updates"));
        CHECK(s.button->isEnabled());

        s.click();
        REQUIRE(browser.opened.size() == 1);
        CHECK(browser.opened.constFirst() == QUrl(QStringLiteral("https://github.com/octo-cat/example/releases/latest")));
    });
}

TEST_CASE("without a public repository the button is off and a click opens nothing", "[updates]")
{
    withScene([](Scene &s, FakeBrowser &browser) {
        s.card->setProperty("releaseUrl", QString());
        CHECK_FALSE(s.button->isEnabled());

        s.click();
        CHECK(browser.opened.isEmpty());
    });
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}
