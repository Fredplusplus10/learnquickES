#pragma once

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QSqlDatabase>
#include <QString>
#include <QStringList>

#include <optional>

namespace lq {

// What the learner tells the app on the welcome screen. The reason is free text in their own words:
// it personalises the plan now and gives the conversation practice context later.
struct Profile {
    QString name;
    QString reason;
    QStringList goals;  // tags picked on the welcome screen: "travel", "work", …
    QString level;      // "A1", "A2" or "A2+", as in the web app
    int planDays = 60;
    QDate planStart;
    QList<int> studyDays{1, 2, 3, 4, 5};  // weekdays, 0 = Sunday
    QDateTime createdAt;
    QString streakEmoji;  // picked on Today, like Snapchat's; empty for the app's own flame
    // Set in the web app and kept from its backups; the desktop app has no screen for them yet.
    bool skipHolidays = true;  // US federal holidays are not study days
    QList<QDate> daysOff;

    bool operator==(const Profile &) const = default;
};

inline constexpr int kMaxNameLength = 40;
inline constexpr int kMinReasonLength = 20;  // a sentence, not a single word
inline constexpr int kMaxReasonLength = 1000;

// Empty when the profile can be saved; otherwise what to fix, in English, ready to show the learner.
QString validateProfile(const Profile &profile);

// The first emoji of `text`, whole (a flag, a skin tone, a family joined with ZWJ, a keycap), or empty when
// `text` does not start with one.
QString firstEmoji(const QString &text);

std::optional<Profile> loadProfile(const QSqlDatabase &db);
bool saveProfile(const QSqlDatabase &db, const Profile &profile, QString *error = nullptr);

} // namespace lq
