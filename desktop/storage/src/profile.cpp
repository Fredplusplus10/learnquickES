#include "lq/profile.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QTextBoundaryFinder>
#include <QVariant>

namespace lq {

namespace {

QString joinDays(const QList<int> &days)
{
    QStringList parts;
    for (int d : days)
        parts << QString::number(d);
    return parts.join(u',');
}

QList<int> splitDays(const QString &text)
{
    QList<int> days;
    for (const QString &part : text.split(u',', Qt::SkipEmptyParts))
        days << part.toInt();
    return days;
}

QString joinDates(const QList<QDate> &dates)
{
    QStringList parts;
    for (const QDate &date : dates)
        parts << date.toString(Qt::ISODate);
    return parts.join(u',');
}

// A date that does not parse is dropped: a day off less is better than no profile.
QList<QDate> splitDates(const QString &text)
{
    QList<QDate> dates;
    for (const QString &part : text.split(u',', Qt::SkipEmptyParts)) {
        if (const QDate date = QDate::fromString(part, Qt::ISODate); date.isValid())
            dates << date;
    }
    return dates;
}

} // namespace

QString validateProfile(const Profile &profile)
{
    const QString name = profile.name.trimmed();
    if (name.isEmpty())
        return QStringLiteral("Please tell us your name.");
    if (name.size() > kMaxNameLength)
        return QStringLiteral("Please use a shorter name (up to %1 characters).").arg(kMaxNameLength);
    const qsizetype reason = profile.reason.trimmed().size();
    if (reason < kMinReasonLength)
        return QStringLiteral("Tell us a little more about why you want to learn Spanish (at least %1 characters).")
            .arg(kMinReasonLength);
    if (reason > kMaxReasonLength)
        return QStringLiteral("Please keep your reason under %1 characters.").arg(kMaxReasonLength);
    if (!QStringList{QStringLiteral("A1"), QStringLiteral("A2"), QStringLiteral("A2+")}.contains(profile.level))
        return QStringLiteral("Please choose your current level.");
    if (profile.planDays != 30 && profile.planDays != 60 && profile.planDays != 90)
        return QStringLiteral("Please choose a 30, 60 or 90-day plan.");
    if (!profile.planStart.isValid())
        return QStringLiteral("The plan needs a start date.");
    if (profile.studyDays.isEmpty())
        return QStringLiteral("Please keep at least one study day in the week.");
    if (!profile.streakEmoji.isEmpty() && firstEmoji(profile.streakEmoji) != profile.streakEmoji)
        return QStringLiteral("Please pick a single emoji for your streak.");
    return {};
}

QString firstEmoji(const QString &text)
{
    const QString trimmed = text.trimmed();
    QTextBoundaryFinder graphemes(QTextBoundaryFinder::Grapheme, trimmed);
    const qsizetype end = graphemes.toNextBoundary();
    // The longest emoji sequences (a family, a subdivision flag) take 11 to 14 UTF-16 units.
    if (end <= 0 || end > 32)
        return {};
    const QString cluster = trimmed.left(end);
    const char32_t first = cluster.toUcs4().front();
    // Emoji are "other symbols" in Unicode; keycaps start with a digit, # or * and end with U+20E3.
    const bool symbol = QChar::category(first) == QChar::Symbol_Other;
    const bool keycap = cluster.contains(QChar(0x20E3));
    return symbol || keycap ? cluster : QString();
}

std::optional<Profile> loadProfile(const QSqlDatabase &db)
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "SELECT name, reason, goals, level, plan_days, plan_start, study_days, created_at, streak_emoji, "
        "skip_holidays, days_off FROM profile WHERE id = 1"));
    if (!query.exec() || !query.next())
        return std::nullopt;
    Profile profile;
    profile.name = query.value(0).toString();
    profile.reason = query.value(1).toString();
    profile.goals = query.value(2).toString().split(u',', Qt::SkipEmptyParts);
    profile.level = query.value(3).toString();
    profile.planDays = query.value(4).toInt();
    profile.planStart = QDate::fromString(query.value(5).toString(), Qt::ISODate);
    profile.studyDays = splitDays(query.value(6).toString());
    profile.createdAt = QDateTime::fromString(query.value(7).toString(), Qt::ISODateWithMs);
    profile.streakEmoji = query.value(8).toString();
    profile.skipHolidays = query.value(9).toBool();
    profile.daysOff = splitDates(query.value(10).toString());
    return profile;
}

bool saveProfile(const QSqlDatabase &db, const Profile &profile, QString *error)
{
    QSqlQuery query(db);
    query.prepare(QStringLiteral(
        "INSERT OR REPLACE INTO profile "
        "(id, name, reason, goals, level, plan_days, plan_start, study_days, created_at, streak_emoji, "
        "skip_holidays, days_off) VALUES (1, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"));
    query.addBindValue(profile.name);
    query.addBindValue(profile.reason);
    query.addBindValue(profile.goals.join(u','));
    query.addBindValue(profile.level);
    query.addBindValue(profile.planDays);
    query.addBindValue(profile.planStart.toString(Qt::ISODate));
    query.addBindValue(joinDays(profile.studyDays));
    query.addBindValue(profile.createdAt.toUTC().toString(Qt::ISODateWithMs));
    query.addBindValue(profile.streakEmoji);
    query.addBindValue(profile.skipHolidays ? 1 : 0);
    query.addBindValue(joinDates(profile.daysOff));
    if (query.exec())
        return true;
    if (error)
        *error = query.lastError().text();
    return false;
}

} // namespace lq
