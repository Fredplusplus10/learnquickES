// The backup file (lq/backup.h), from Settings: the whole learner in one JSON file that the web app opens too.
#include "learnercontroller.h"
#include "qtdate.h"

#include "lq/backup.h"
#include "lq/learner.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <QStandardPaths>

namespace {

// Far above any real learner (the sample one is about 60 KB); anything bigger is not a backup.
constexpr qint64 kMaxBackupBytes = 8 * 1024 * 1024;

QString qs(const std::string &s)
{
    return QString::fromStdString(s);
}

QString shown(const QUrl &file)
{
    return QDir::toNativeSeparators(file.toLocalFile());
}

} // namespace

QUrl LearnerController::backupSuggestion() const
{
    const QString folder = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return QUrl::fromLocalFile(
        QDir(folder).filePath(QStringLiteral("learnquick-backup-%1.json").arg(m_today.toString(Qt::ISODate))));
}

QString LearnerController::exportBackup(const QUrl &file) const
{
    const auto &profile = m_profile->profile();
    if (!m_state || !m_course || !profile)
        return QStringLiteral("There is nothing to save yet.");
    lq::Backup backup;
    backup.profile.name = profile->name.toStdString();
    backup.profile.level = profile->level.toStdString();
    backup.profile.desktop = true;
    backup.profile.reason = profile->reason.toStdString();
    for (const QString &goal : profile->goals)
        backup.profile.goals.push_back(goal.toStdString());
    backup.profile.streakEmoji = profile->streakEmoji.toStdString();
    backup.state = *m_state;
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    const std::string text = lq::writeBackup(backup, *m_course, now.toStdString());

    // QSaveFile writes beside the target and renames at the end: a failed save never leaves half a file.
    QSaveFile out(file.toLocalFile());
    if (!out.open(QIODevice::WriteOnly)
        || out.write(text.data(), static_cast<qint64>(text.size())) != static_cast<qint64>(text.size())
        || !out.commit())
        return QStringLiteral("The backup could not be saved to %1: %2").arg(shown(file), out.errorString());
    return {};
}

QString LearnerController::importBackup(const QUrl &file)
{
    const auto &current = m_profile->profile();
    if (m_demo)
        return QStringLiteral("The sample learner cannot be replaced. Open LearnQuick without --demo to restore a backup.");
    if (!m_database || !m_course || !current)
        return QStringLiteral("Your learner data is not available, so a backup cannot be restored.");

    QFile in(file.toLocalFile());
    if (!in.open(QIODevice::ReadOnly))
        return QStringLiteral("%1 could not be opened: %2").arg(shown(file), in.errorString());
    // size() is 0 for a pipe or a device, so the limit is also kept while reading: one byte past it is enough
    // to know. Every file within the limit is read whole, as before.
    const QByteArray text = in.size() > kMaxBackupBytes ? QByteArray() : in.read(kMaxBackupBytes + 1);
    if (in.size() > kMaxBackupBytes || text.size() > kMaxBackupBytes)
        return QStringLiteral("This file is too large to be a LearnQuick backup.");
    std::string why;
    const auto backup = lq::readBackup(std::string_view(text.constData(), static_cast<std::size_t>(text.size())),
                                       *m_course, &why);
    if (!backup)
        return qs(why);

    // A backup from the web app has no reason, goals or streak emoji: the learner keeps theirs.
    lq::Profile profile = *current;
    if (!backup->profile.name.empty())
        profile.name = qs(backup->profile.name);
    if (!backup->profile.level.empty())
        profile.level = qs(backup->profile.level);
    if (backup->profile.desktop) {
        if (!backup->profile.reason.empty())
            profile.reason = qs(backup->profile.reason);
        profile.goals.clear();
        for (const std::string &goal : backup->profile.goals)
            profile.goals.append(qs(goal));
        profile.streakEmoji = qs(backup->profile.streakEmoji);
    }
    const lq::Plan &plan = backup->state.plan;
    profile.planDays = plan.days;
    profile.planStart = toQDate(plan.start);
    profile.studyDays = QList<int>(plan.studyDays.begin(), plan.studyDays.end());
    profile.skipHolidays = plan.skipHolidays;
    profile.daysOff.clear();
    for (const lq::Date date : plan.daysOff)
        profile.daysOff.append(toQDate(date));
    if (const QString problem = lq::validateProfile(profile); !problem.isEmpty())
        return QStringLiteral("This backup cannot be restored: %1").arg(problem);

    QString error;
    if (!lq::restoreLearner(m_database->connection(), profile, backup->state, &error))
        return QStringLiteral("The backup could not be restored, so nothing changed: %1").arg(error);
    // The profile re-reads what was stored, and its change reloads the progress (load()).
    m_profile->reload();
    return {};
}
