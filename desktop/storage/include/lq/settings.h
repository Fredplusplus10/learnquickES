#pragma once

#include "lq/speech.h"

#include <QSqlDatabase>
#include <QString>

namespace lq {

// What the learner chooses on the Settings screen. Stored as one row per setting, so a new one needs no
// migration; anything missing or out of range reads as its default.
struct Settings {
    QString theme = QStringLiteral("system");  // "light" (day), "dark" (night) or "system", as the computer is
    QString voice = QString::fromLatin1(kNarratorVoice.data(), static_cast<qsizetype>(kNarratorVoice.size()));
    double pace = kDefaultPace;
    double slowPace = kDefaultSlowPace;

    bool operator==(const Settings &) const = default;
};

// The same settings brought into range: an unknown theme or voice becomes the default, paces are clamped to
// their ranges, and slow stays at least kSlowPaceGap slower than normal.
Settings normalized(Settings settings);

Settings loadSettings(const QSqlDatabase &db);
bool saveSettings(const QSqlDatabase &db, const Settings &settings, QString *error = nullptr);

} // namespace lq
