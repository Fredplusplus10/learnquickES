#include "preferencescontroller.h"

#include <QJSEngine>
#include <QPointer>
#include <QVariantMap>

#include <type_traits>

static_assert(!std::is_default_constructible_v<PreferencesController>, "QML would make its own instance");

namespace {

QPointer<PreferencesController> s_instance;

QString text(std::string_view view)
{
    return QString::fromUtf8(view.data(), static_cast<qsizetype>(view.size()));
}

} // namespace

void PreferencesController::setInstance(PreferencesController *instance)
{
    s_instance = instance;
}

PreferencesController *PreferencesController::create(QQmlEngine *, QJSEngine *engine)
{
    if (!s_instance)
        return new PreferencesController(nullptr);  // owned by the engine
    Q_ASSERT(engine->thread() == s_instance->thread());
    QJSEngine::setObjectOwnership(s_instance, QJSEngine::CppOwnership);
    return s_instance;
}

PreferencesController::PreferencesController(QObject *parent)
    : QObject(parent)
{
}

void PreferencesController::attach(lq::Database *database)
{
    m_database = database;
    m_settings = lq::loadSettings(database->connection());
    emit changed();
}

void PreferencesController::preview(const lq::Settings &settings)
{
    m_database = nullptr;
    m_settings = lq::normalized(settings);
    emit changed();
}

bool PreferencesController::recommendedVoice() const
{
    const lq::Settings defaults;
    return m_settings.voice == defaults.voice && m_settings.pace == defaults.pace
           && m_settings.slowPace == defaults.slowPace;
}

QVariantList PreferencesController::voices() const
{
    QVariantList list;
    for (const lq::VoiceChoice &choice : lq::voiceChoices()) {
        list.append(QVariantMap{
            {QStringLiteral("style"), text(choice.style)},
            {QStringLiteral("name"), text(choice.name)},
            {QStringLiteral("detail"), text(choice.detail)},
        });
    }
    return list;
}

void PreferencesController::setTheme(const QString &theme)
{
    lq::Settings next = m_settings;
    next.theme = theme;
    apply(next);
}

void PreferencesController::setVoice(const QString &style)
{
    lq::Settings next = m_settings;
    next.voice = style;
    apply(next);
}

void PreferencesController::setPace(double pace)
{
    lq::Settings next = m_settings;
    next.pace = pace;
    apply(next);
}

void PreferencesController::setSlowPace(double pace)
{
    lq::Settings next = m_settings;
    next.slowPace = pace;
    apply(next);
}

void PreferencesController::resetVoice()
{
    lq::Settings next;
    next.theme = m_settings.theme;
    apply(next);
}

double PreferencesController::syllablesPerSecond(double pace) const
{
    return pace > 0 ? lq::kReferenceSyllablesPerSecond / pace : 0.0;
}

void PreferencesController::apply(const lq::Settings &settings)
{
    const lq::Settings next = lq::normalized(settings);
    if (next == m_settings)
        return;
    QString dbError;
    if (m_database && !lq::saveSettings(m_database->connection(), next, &dbError)) {
        setError(QStringLiteral("Your settings could not be saved: %1").arg(dbError));
        return;
    }
    setError({});
    m_settings = next;
    emit changed();
}

void PreferencesController::setError(const QString &error)
{
    if (error == m_error)
        return;
    m_error = error;
    emit errorChanged();
}
