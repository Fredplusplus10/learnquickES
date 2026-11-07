#include "profilecontroller.h"

#include "lq/demo.h"
#include "lq/progress.h"
#include "qtdate.h"

#include <QTimeZone>

ProfileController::ProfileController(QDate today, QObject *parent)
    : QObject(parent)
    , m_today(today)
{
}

void ProfileController::attach(lq::Database *database)
{
    m_database = database;
    m_profile = lq::loadProfile(database->connection());
    emit changed();
}

void ProfileController::reload()
{
    if (m_database)
        attach(m_database);
}

void ProfileController::preview(const lq::Profile &profile)
{
    m_database = nullptr;
    m_previewing = true;
    m_profile = profile;
    emit changed();
}

void ProfileController::setError(const QString &error)
{
    if (error == m_error)
        return;
    m_error = error;
    emit errorChanged();
}

int ProfileController::recommendedPlanDays(const QString &level) const
{
    return lq::recommendedPlanDays(level.toStdString());
}

bool ProfileController::save(const QString &name, const QString &reason, const QStringList &goals, const QString &level,
                             int planDays)
{
    // Changing the welcome answers keeps everything else: study days, streak emoji, days off.
    lq::Profile profile = m_profile.value_or(lq::Profile{});
    profile.name = name.trimmed();
    profile.reason = reason.trimmed();
    profile.goals = goals;
    profile.level = level;
    profile.planDays = planDays;
    // Changing your answers later must not restart a plan already under way.
    profile.planStart = m_profile ? m_profile->planStart : m_today;
    profile.createdAt = m_profile ? m_profile->createdAt : QDateTime::currentDateTimeUtc();

    if (!m_database && !m_previewing) {
        // The database failed to open at startup; that error is already showing.
        if (m_error.isEmpty())
            setError(QStringLiteral("Your answers cannot be saved: the learner database is not available."));
        return false;
    }
    if (const QString problem = lq::validateProfile(profile); !problem.isEmpty()) {
        setError(problem);
        return false;
    }
    QString dbError;
    if (m_database && !lq::saveProfile(m_database->connection(), profile, &dbError)) {
        setError(QStringLiteral("Your answers could not be saved: %1").arg(dbError));
        return false;
    }
    setError({});
    m_profile = profile;
    emit changed();
    return true;
}

bool ProfileController::setStreakEmoji(const QString &text)
{
    if (!m_profile)
        return false;
    const QString emoji = lq::firstEmoji(text);
    if (emoji.isEmpty() && !text.trimmed().isEmpty())
        return false;
    lq::Profile profile = *m_profile;
    profile.streakEmoji = emoji;
    QString dbError;
    if (m_database && !lq::saveProfile(m_database->connection(), profile, &dbError)) {
        setError(QStringLiteral("Your streak emoji could not be saved: %1").arg(dbError));
        return false;
    }
    setError({});
    m_profile = profile;
    emit changed();
    return true;
}

lq::Profile ProfileController::demoProfile(QDate today)
{
    lq::Profile profile;
    profile.name = QStringLiteral("Cory");
    profile.reason = QStringLiteral("I travel to Mexico for work a few times a year and I want real conversations "
                                    "with my team there, not just ordering food.");
    profile.goals = {QStringLiteral("work"), QStringLiteral("travel")};
    profile.level = QStringLiteral("A2");
    profile.planDays = 60;
    profile.planStart = toQDate(lq::demoState(toLqDate(today), {}).plan.start);
    profile.createdAt = QDateTime(profile.planStart, QTime(9, 0), QTimeZone::UTC);
    return profile;
}
