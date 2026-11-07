#include "lq/settings.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <algorithm>
#include <cmath>
#include <utility>

namespace lq {

namespace {

bool knownVoice(const QString &style)
{
    const std::string name = style.toStdString();
    return std::ranges::any_of(voiceChoices(), [&](const VoiceChoice &choice) { return choice.style == name; });
}

} // namespace

Settings normalized(Settings settings)
{
    const Settings defaults;
    if (settings.theme != QStringLiteral("light") && settings.theme != QStringLiteral("dark"))
        settings.theme = defaults.theme;
    if (!knownVoice(settings.voice))
        settings.voice = defaults.voice;
    // Steps of 0.05, as the sliders move: a value stored by hand reads back as one the screen can show.
    const auto step = [](double value) { return std::round(value * 20.0) / 20.0; };
    settings.pace = std::isfinite(settings.pace) ? std::clamp(step(settings.pace), kMinPace, kMaxPace) : defaults.pace;
    if (!std::isfinite(settings.slowPace))
        settings.slowPace = defaults.slowPace;
    settings.slowPace = std::clamp(step(std::max(settings.slowPace, settings.pace + kSlowPaceGap)), kMinSlowPace,
                                   kMaxSlowPace);
    return settings;
}

Settings loadSettings(const QSqlDatabase &db)
{
    Settings settings;
    QSqlQuery query(db);
    if (!query.exec(QStringLiteral("SELECT key, value FROM setting")))
        return settings;
    while (query.next()) {
        const QString key = query.value(0).toString();
        const QString value = query.value(1).toString();
        bool ok = false;
        if (key == QStringLiteral("theme")) {
            settings.theme = value;
        } else if (key == QStringLiteral("voice")) {
            settings.voice = value;
        } else if (key == QStringLiteral("pace")) {
            if (const double pace = value.toDouble(&ok); ok)
                settings.pace = pace;
        } else if (key == QStringLiteral("slow_pace")) {
            if (const double pace = value.toDouble(&ok); ok)
                settings.slowPace = pace;
        }
    }
    return normalized(settings);
}

bool saveSettings(const QSqlDatabase &db, const Settings &settings, QString *error)
{
    const Settings clean = normalized(settings);
    const std::pair<QString, QString> rows[] = {
        {QStringLiteral("theme"), clean.theme},
        {QStringLiteral("voice"), clean.voice},
        {QStringLiteral("pace"), QString::number(clean.pace, 'f', 2)},
        {QStringLiteral("slow_pace"), QString::number(clean.slowPace, 'f', 2)},
    };
    // All or nothing: a half-saved change would mix two choices.
    QSqlDatabase connection = db;
    if (!connection.transaction()) {
        if (error)
            *error = connection.lastError().text();
        return false;
    }
    QSqlQuery query(connection);
    query.prepare(QStringLiteral("INSERT OR REPLACE INTO setting (key, value) VALUES (?, ?)"));
    for (const auto &[key, value] : rows) {
        query.addBindValue(key);
        query.addBindValue(value);
        if (!query.exec()) {
            if (error)
                *error = query.lastError().text();
            connection.rollback();
            return false;
        }
    }
    if (connection.commit())
        return true;
    if (error)
        *error = connection.lastError().text();
    return false;
}

} // namespace lq
