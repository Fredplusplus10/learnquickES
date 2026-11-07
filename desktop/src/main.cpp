#include "certificatecontroller.h"
#include "learnercontroller.h"
#include "lq/database.h"
#include "preferencescontroller.h"
#include "profilecontroller.h"
#include "sessionmodel.h"
#include "speechcontroller.h"
#include "todaymodel.h"

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickWindow>
#include <QSize>
#include <QStandardPaths>
#include <QTimer>

#include <cstdlib>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("LearnQuick"));
    QGuiApplication::setOrganizationName(QStringLiteral("LearnQuick"));
    QGuiApplication::setApplicationVersion(QStringLiteral(LQ_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Spanish to functional B1 in 30 to 90 days."));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption smokeTest(QStringLiteral("smoke-test"),
                                       QStringLiteral("Load the interface, then quit. Fails on any QML warning."));
    parser.addOption(smokeTest);
    const QCommandLineOption screenshot(QStringLiteral("screenshot"),
                                        QStringLiteral("Save a PNG of the main window to <file>, then quit."),
                                        QStringLiteral("file"));
    parser.addOption(screenshot);
    const QCommandLineOption demo(QStringLiteral("demo"),
                                  QStringLiteral("Show a sample learner three weeks into the plan. Nothing is saved."));
    parser.addOption(demo);
    const QCommandLineOption todayOption(QStringLiteral("today"),
                                         QStringLiteral("Use <date> (YYYY-MM-DD) as today, for reproducible screenshots."),
                                         QStringLiteral("date"));
    parser.addOption(todayOption);
    const QCommandLineOption dataDir(QStringLiteral("data-dir"),
                                     QStringLiteral("Keep the learner database in <dir> instead of the app data folder."),
                                     QStringLiteral("dir"));
    parser.addOption(dataDir);
    const QCommandLineOption welcomeStep(QStringLiteral("welcome-step"),
                                         QStringLiteral("Open the welcome flow at step <n> (1-3), for screenshots."),
                                         QStringLiteral("n"), QStringLiteral("1"));
    parser.addOption(welcomeStep);
    const QCommandLineOption open(QStringLiteral("open"),
                                  QStringLiteral("Open <route> instead of Today, e.g. lessons, lesson/3/practice, practice/verbs, "
                                                 "checkpoint/1, mock/writing or settings. For screenshots."),
                                  QStringLiteral("route"), QStringLiteral("today"));
    parser.addOption(open);
    const QCommandLineOption requireSpeech(QStringLiteral("require-speech"),
                                           QStringLiteral("With --smoke-test: also fail when the built-in voice does "
                                                          "not speak or no system speech engine was deployed."));
    parser.addOption(requireSpeech);
    const QCommandLineOption themeOption(QStringLiteral("theme"),
                                         QStringLiteral("Show the app in <theme> (light or dark) without saving it, "
                                                        "for screenshots."),
                                         QStringLiteral("theme"));
    parser.addOption(themeOption);
    const QCommandLineOption sizeOption(QStringLiteral("size"),
                                        QStringLiteral("Open the window at <size> pixels, e.g. 1440x900, for screenshots."),
                                        QStringLiteral("size"));
    parser.addOption(sizeOption);
    parser.process(app);

    QSize windowSize;
    if (parser.isSet(sizeOption)) {
        const QStringList parts = parser.value(sizeOption).split(u'x');
        if (parts.size() == 2)
            windowSize = QSize(parts[0].toInt(), parts[1].toInt());
        if (windowSize.width() < 900 || windowSize.height() < 600) {
            qCritical("--size expects a size such as 1440x900, at least 900x600.");
            return EXIT_FAILURE;
        }
    }

    QDate today = QDate::currentDate();
    if (parser.isSet(todayOption)) {
        today = QDate::fromString(parser.value(todayOption), Qt::ISODate);
        if (!today.isValid()) {
            qCritical("--today expects a date such as 2026-11-02.");
            return EXIT_FAILURE;
        }
    }

    // Declared before the engine, so QML never outlives the objects it binds to.
    lq::Database database;
    PreferencesController preferences(nullptr);
    PreferencesController::setInstance(&preferences);
    ProfileController profile(today);
    LearnerController learner(&profile, today, parser.isSet(demo), LearnerController::bundledCoursePath());
    CertificateController certificates(nullptr);
    CertificateController::setInstance(&certificates);
    certificates.setProfile(&profile);
    certificates.setToday(today);
    if (parser.isSet(demo)) {
        profile.preview(ProfileController::demoProfile(today));
    } else {
        const QString dir = parser.isSet(dataDir) ? parser.value(dataDir)
                                                  : QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        if (QDir().mkpath(dir) && database.open(QDir(dir).filePath(QStringLiteral("learnquick.sqlite3")))) {
            preferences.attach(&database);
            profile.attach(&database);
            learner.attach(&database);
            certificates.attach(&database, dir);
        } else {
            profile.setError(QStringLiteral("Your learner data in %1 could not be opened. %2")
                                 .arg(QDir::toNativeSeparators(dir), database.error()));
        }
    }
    if (parser.isSet(themeOption)) {
        lq::Settings settings = preferences.settings();
        settings.theme = parser.value(themeOption);
        preferences.preview(settings);
    }
    TodayModel todayModel(&learner);
    SpeechController speech(nullptr);
    SpeechController::setInstance(&speech);
    // The learner's voice and paces from Settings, now and whenever they change.
    const auto applyVoice = [&] {
        speech.setNarrator(preferences.voice());
        speech.setPace(preferences.pace(), preferences.slowPace());
    };
    applyVoice();
    QObject::connect(&preferences, &PreferencesController::changed, &speech, applyVoice);
    // Smoke tests and screenshots open screens that speak by themselves; CI has nobody to listen.
    if (parser.isSet(smokeTest) || parser.isSet(screenshot))
        speech.setMuted(true);
    else
        speech.preload();
    SessionModel session(&learner);
    session.setAudioAvailable(speech.available());
    QObject::connect(&speech, &SpeechController::availableChanged, &session,
                     [&] { session.setAudioAvailable(speech.available()); });
    QObject::connect(&session, &SessionModel::sayRequested, &speech, [&](const QString &text) { speech.say(text); });

    QQmlApplicationEngine engine;
    bool qmlWarnings = false;
    QObject::connect(&engine, &QQmlEngine::warnings, &app,
                     [&qmlWarnings](const QList<QQmlError> &) { qmlWarnings = true; });

    engine.setInitialProperties({
        {QStringLiteral("profile"), QVariant::fromValue(&profile)},
        {QStringLiteral("today"), QVariant::fromValue(&todayModel)},
        {QStringLiteral("learner"), QVariant::fromValue(&learner)},
        {QStringLiteral("session"), QVariant::fromValue(&session)},
        {QStringLiteral("initialRoute"), parser.value(open)},
        {QStringLiteral("welcomeStep"), qBound(1, parser.value(welcomeStep).toInt(), 3) - 1},
    });
    engine.loadFromModule("LearnQuick", "Main");
    if (engine.rootObjects().isEmpty())
        return EXIT_FAILURE;
    if (windowSize.isValid()) {
        if (auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst()))
            window->resize(windowSize);
    }

    if (parser.isSet(smokeTest)) {
        // CI runs this on the build tree and on the deployed app: it proves the QML module, its imports, the Qt
        // plugins (SQLite and text-to-speech included) and the course data load; with --require-speech, the voice.
        QTimer::singleShot(0, &app, &QCoreApplication::quit);
        const int code = app.exec();
        for (const QString &error : {profile.error(), learner.error(), preferences.error()}) {
            if (!error.isEmpty()) {
                qCritical("%s", qUtf8Printable(error));
                return EXIT_FAILURE;
            }
        }
        // The screens must talk to the objects set up here: a copy of their own would neither save the
        // settings nor speak with the learner's voice.
        if (engine.singletonInstance<PreferencesController *>("LearnQuick", "Preferences") != &preferences
            || engine.singletonInstance<SpeechController *>("LearnQuick", "Speech") != &speech
            || engine.singletonInstance<CertificateController *>("LearnQuick", "Certificates") != &certificates) {
            qCritical("QML made its own Preferences, Speech or Certificates instead of using the application's.");
            return EXIT_FAILURE;
        }
        if (parser.isSet(requireSpeech)) {
            if (SpeechController::engines().isEmpty()) {
                qCritical("No text-to-speech engine was deployed with the app.");
                return EXIT_FAILURE;
            }
#ifdef LQ_NEURAL_VOICE
            if (QString error; !speech.checkBundledVoice(&error)) {
                qCritical("The built-in voice does not work: %s", qUtf8Printable(error));
                return EXIT_FAILURE;
            }
#endif
        }
        return qmlWarnings ? EXIT_FAILURE : code;
    }

    if (parser.isSet(screenshot)) {
        // CI saves one per platform, so the macOS build can be seen without a Mac.
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().constFirst());
        if (!window)
            return EXIT_FAILURE;
        const QString path = parser.value(screenshot);
        // Grab after the first rendered frame: before it, the window may not have its final size and layout.
        QObject::connect(window, &QQuickWindow::frameSwapped, &app, [window, path] {
            QTimer::singleShot(0, window, [window, path] {
                QCoreApplication::exit(window->grabWindow().save(path) ? EXIT_SUCCESS : EXIT_FAILURE);
            });
        }, Qt::SingleShotConnection);
        // No frame at all: fail instead of hanging CI.
        QTimer::singleShot(10'000, &app, [] { QCoreApplication::exit(EXIT_FAILURE); });
        const int code = app.exec();
        // Each screenshot opens a different screen, so a broken binding anywhere fails CI.
        return qmlWarnings ? EXIT_FAILURE : code;
    }

    return app.exec();
}
