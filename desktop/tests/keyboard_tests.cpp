// The on-screen keyboard, clicked the way a learner clicks it: it types where the cursor is without taking the
// focus, so the physical keyboard keeps working, and its Enter checks the answer.
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>

#include <QFile>
#include <QGuiApplication>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QTemporaryDir>
#include <QTest>
#include <QtQuickTest/quicktest.h>

#include <functional>
#include <string>

// Failures print the text itself instead of {?}.
namespace Catch {
template <>
struct StringMaker<QString> {
    static std::string convert(const QString &text) { return '"' + text.toStdString() + '"'; }
};
} // namespace Catch

namespace {

// An answer box with its keyboard, and the mock exam's essay box with its own.
constexpr char kScene[] = R"(
import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

Rectangle {
    property int submits: 0
    width: 860
    height: 760

    ColumnLayout {
        x: 24
        y: 24
        width: 800
        spacing: 14

        TextField {
            id: field
            objectName: "field"
            Layout.preferredWidth: 360
        }
        SpanishKeyboard {
            objectName: "keyboard"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            target: field
            enterText: field.readOnly ? "Continue" : "Check"
            enterEnabled: field.readOnly || field.text !== ""
            onSubmitted: submits++
        }
        TextArea {
            id: essay
            objectName: "essay"
            Layout.preferredWidth: 400
            Layout.preferredHeight: 60
        }
        SpanishKeyboard {
            objectName: "essayKeyboard"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            target: essay
            multiline: true
        }
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

// The keyboard and what it uses, outside the app's QML module: the source files next to a qmldir.
bool writeScene(const QTemporaryDir &dir)
{
    for (const char *name : {"SpanishKeyboard.qml", "Icon.qml", "Theme.qml"}) {
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

// A key by what it types or does ("q", "ñ", "shift", "enter"...); the row of Spanish letters comes first.
QQuickItem *findKey(QQuickItem *item, const QString &name)
{
    for (QQuickItem *child : item->childItems()) {
        if (child->metaObject()->indexOfProperty("autoRepeat") >= 0 && child->property("modelData").toString() == name)
            return child;
        if (QQuickItem *found = findKey(child, name))
            return found;
    }
    return nullptr;
}

struct Scene {
    QStringList warnings;  // before the view, which can still warn while it is destroyed
    QQuickView view;
    QQuickItem *root = nullptr;

    QQuickItem *item(const char *name) const { return root->findChild<QQuickItem *>(QLatin1String(name)); }
    QString text(const char *name) const { return item(name)->property("text").toString(); }

    QPoint centre(const char *keyboard, const QString &key)
    {
        // Switching to numbers or back builds new keys; the layouts place them before they can be clicked.
        REQUIRE(QQuickTest::qWaitForPolish(&view));
        QQuickItem *button = findKey(item(keyboard), key);
        REQUIRE(button != nullptr);
        return button->mapToScene(QPointF(button->width() / 2, button->height() / 2)).toPoint();
    }

    void click(const char *keyboard, std::initializer_list<const char *> keys)
    {
        for (const char *key : keys) {
            QTest::mouseClick(&view, Qt::LeftButton, {}, centre(keyboard, QString::fromUtf8(key)));
        }
    }
};

void withScene(const std::function<void(Scene &)> &test)
{
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    REQUIRE(writeScene(dir));
    Scene scene;
    QObject::connect(scene.view.engine(), &QQmlEngine::warnings, [&scene](const QList<QQmlError> &warnings) {
        for (const QQmlError &warning : warnings)
            scene.warnings << warning.toString();
    });
    scene.view.setSource(QUrl::fromLocalFile(dir.filePath(QStringLiteral("Scene.qml"))));
    INFO(scene.warnings.join(QLatin1Char('\n')).toStdString());
    REQUIRE(scene.view.status() == QQuickView::Ready);
    scene.root = scene.view.rootObject();
    scene.view.show();
    REQUIRE(QTest::qWaitForWindowExposed(&scene.view));
    scene.item("field")->forceActiveFocus();
    test(scene);
    INFO(scene.warnings.join(QLatin1Char('\n')).toStdString());
    CHECK(scene.warnings.isEmpty());
}

} // namespace

TEST_CASE("the on-screen keyboard types Spanish where the cursor is and leaves the focus in the answer", "[keyboard]")
{
    withScene([](Scene &s) {
        s.click("keyboard", {"¿", "shift", "q", "u", "é", "space", "t", "a", "l", "?"});
        CHECK(s.text("field") == QString::fromUtf8("¿Qué tal?"));
        CHECK(s.item("field")->hasActiveFocus());

        s.click("keyboard", {"delete"});
        CHECK(s.text("field") == QString::fromUtf8("¿Qué tal"));

        for (char c : std::string(" bien"))
            QTest::keyClick(&s.view, c);
        CHECK(s.text("field") == QString::fromUtf8("¿Qué tal bien"));

        // Shift twice locks capitals, Ñ included; a third time unlocks.
        s.item("field")->setProperty("cursorPosition", 1);
        s.click("keyboard", {"shift", "shift", "a", "ñ"});
        CHECK(s.text("field") == QString::fromUtf8("¿AÑQué tal bien"));
        s.click("keyboard", {"shift", "o"});
        CHECK(s.text("field") == QString::fromUtf8("¿AÑoQué tal bien"));

        QMetaObject::invokeMethod(s.item("field"), "selectAll");
        s.click("keyboard", {"ü"});
        CHECK(s.text("field") == QString::fromUtf8("ü"));

        s.click("keyboard", {"symbols", "1", "0", "€", "letters", "s"});
        CHECK(s.text("field") == QString::fromUtf8("ü10€s"));
    });
}

TEST_CASE("the on-screen Enter checks the answer, then moves on once it is locked", "[keyboard]")
{
    withScene([](Scene &s) {
        s.click("keyboard", {"enter"});
        CHECK(s.root->property("submits").toInt() == 0);  // nothing typed yet

        s.click("keyboard", {"s", "o", "y", "enter"});
        CHECK(s.root->property("submits").toInt() == 1);

        // A checked answer is read-only: letters do nothing, Enter continues.
        s.item("field")->setProperty("readOnly", true);
        s.click("keyboard", {"a", "enter"});
        CHECK(s.text("field") == QStringLiteral("soy"));
        CHECK(s.root->property("submits").toInt() == 2);
    });
}

TEST_CASE("in the essay Enter starts a new line, and holding delete keeps deleting", "[keyboard]")
{
    withScene([](Scene &s) {
        s.click("essayKeyboard", {"h", "o", "l", "a", "enter", "y"});
        CHECK(s.text("essay") == QStringLiteral("hola\ny"));

        const QPoint key = s.centre("essayKeyboard", QStringLiteral("delete"));
        QTest::mousePress(&s.view, Qt::LeftButton, {}, key);
        QTest::qWait(1500);
        QTest::mouseRelease(&s.view, Qt::LeftButton, {}, key);
        CHECK(s.text("essay").isEmpty());
    });
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    return Catch::Session().run(argc, argv);
}
