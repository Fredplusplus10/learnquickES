#include "todaymodel.h"

#include "lq/progress.h"
#include "qtdate.h"

#include <QLocale>
#include <QVariantMap>

#include <algorithm>
#include <cstdlib>

namespace {

QString qs(const std::string &s)
{
    return QString::fromStdString(s);
}

// Same wording as the web app's Today screen (app/js/views/home.js), English only.
struct TaskText {
    QString name, detail, tone;
};

TaskText taskText(lq::Task task, const lq::ScheduleEntry &entry, const lq::Course &course, int due)
{
    using lq::Task;
    switch (task) {
    case Task::Review:
        return {QStringLiteral("Review"),
                due ? QStringLiteral("%1 cards due · spaced repetition").arg(due) : QStringLiteral("Nothing due today"),
                QStringLiteral("blue")};
    case Task::Lesson: {
        const lq::Unit &unit = course.units.at(static_cast<std::size_t>(entry.unit));
        return {QStringLiteral("Lesson %1: %2").arg(unit.n).arg(qs(unit.title)), qs(unit.canDo), QStringLiteral("accent")};
    }
    case Task::Verbs:
        return {QStringLiteral("Verb gym · 80/20"), QStringLiteral("10 conjugations of the key verbs"), QStringLiteral("accent")};
    case Task::Mixed:
        return {QStringLiteral("Mixed practice"), QStringLiteral("Exercises from the units you have done"), QStringLiteral("purple")};
    case Task::Speak:
        return {QStringLiteral("Speaking: shadowing"), QStringLiteral("Listen and repeat out loud"), QStringLiteral("green")};
    case Task::Cognates:
        return {QStringLiteral("COGNATES LAB"), QStringLiteral("-tion becomes -ción: words you already know"), QStringLiteral("accent")};
    case Task::Gender:
        return {QStringLiteral("El / La trainer"), QStringLiteral("Learn every noun with its article"), QStringLiteral("purple")};
    case Task::Past:
        return {QStringLiteral("Fui vs. Iba"), QStringLiteral("The real B1 filter: narrating the past"), QStringLiteral("blue")};
    case Task::FalseFriends:
        return {QStringLiteral("False friends"), QStringLiteral("Words that look English but are not"), QStringLiteral("accent")};
    case Task::Describe:
        return {QStringLiteral("Describe it in Spanish"), QStringLiteral("Language-island skill: no English allowed"), QStringLiteral("green")};
    case Task::Checkpoint:
        return {QStringLiteral("Checkpoint · Phase %1").arg(entry.phase), QStringLiteral("15 questions from this phase"), QStringLiteral("accent")};
    case Task::Final:
        return {QStringLiteral("B1 mock exam"), QStringLiteral("Reading, listening, grammar, writing and speaking"), QStringLiteral("accent")};
    }
    return {};
}

// Like the engine's week strip, but days before the plan started are neither missed nor off.
QString dayState(lq::Date day, lq::Date today, const lq::LearnerState &state, const std::set<lq::Date> &active)
{
    if (active.contains(day))
        return QStringLiteral("done");
    if (state.frozen.contains(day))
        return QStringLiteral("frozen");
    if (day > today)
        return QStringLiteral("future");
    if (day == today)
        return QStringLiteral("today");
    if (day < state.plan.start)
        return QStringLiteral("before");
    return lq::isStudyDay(day, state.plan) ? QStringLiteral("missed") : QStringLiteral("off");
}

QString days(int n)
{
    return n == 1 ? QStringLiteral("1 day") : QStringLiteral("%1 days").arg(n);
}

// Today shows the coach's first few suggestions; the coach screen has them all.
constexpr qsizetype kCoachTips = 3;

} // namespace

TodayModel::TodayModel(LearnerController *learner, QObject *parent)
    : QObject(parent)
    , m_learner(learner)
{
    connect(m_learner, &LearnerController::changed, this, &TodayModel::refresh);
    refresh();
}

int TodayModel::maxFreezes() const
{
    return lq::kMaxFreezes;
}

void TodayModel::refresh()
{
    View v;
    const auto &profile = m_learner->profile()->profile();
    const lq::Course *coursePtr = m_learner->course();
    if (!profile || !coursePtr || !m_learner->ready()) {
        m_view = v;
        emit changed();
        return;
    }
    const lq::Course &course = *coursePtr;
    const lq::LearnerState &state = m_learner->state();
    const QLocale en(QLocale::English, QLocale::UnitedStates);
    const QDate qtoday = m_learner->today();
    const lq::Date today = m_learner->day();

    v.learnerName = profile->name;
    v.reason = profile->reason;
    v.dateText = en.toString(qtoday, QStringLiteral("dddd, MMMM d"));

    // The plan for today.
    const lq::PlanStatus status = lq::planStatus(state);
    const lq::ScheduleEntry &entry = status.entry;
    v.dayNumber = status.n;
    v.totalDays = status.total;
    v.finished = status.finished;
    v.finishedToday = std::any_of(state.done.begin(), state.done.end(), [today](const lq::DoneDay &d) { return d.date == today; });
    v.phase = entry.phase;
    v.phaseTitle = qs(course.phases.at(static_cast<std::size_t>(entry.phase - 1)).title);
    v.minutes = status.minutes;
    v.inputMinutes = lq::inputTargetMinutes(status.total);
    switch (entry.type) {
    case lq::DayType::Lesson:
        v.dayKind = QStringLiteral("Lesson day");
        v.dayTitle = qs(course.units.at(static_cast<std::size_t>(entry.unit)).title);
        v.dayTitleEs = qs(course.units.at(static_cast<std::size_t>(entry.unit)).titleEs);
        break;
    case lq::DayType::Practice:
        v.dayKind = QStringLiteral("Practice day");
        v.dayTitle = QStringLiteral("Practice and review");
        v.dayTitleEs = QStringLiteral("Práctica y repaso");
        break;
    case lq::DayType::Checkpoint:
        if (entry.phase == lq::kPhases) {
            v.dayKind = QStringLiteral("Final exam");
            v.dayTitle = QStringLiteral("B1 mock exam");
            v.dayTitleEs = QStringLiteral("Simulacro B1");
        } else {
            v.dayKind = QStringLiteral("Checkpoint");
            v.dayTitle = QStringLiteral("Phase %1 checkpoint").arg(entry.phase);
            v.dayTitleEs = QStringLiteral("Control de la fase %1").arg(entry.phase);
        }
        break;
    }
    if (status.finished) {
        v.dayKind = QStringLiteral("Plan complete");
        v.dayTitle = QStringLiteral("You finished the plan");
        v.dayTitleEs = QStringLiteral("¡Lo lograste!");
    }
    const int due = m_learner->dueCount();
    for (const lq::Task task : status.tasks) {
        const TaskText text = taskText(task, entry, course, due);
        const std::string_view id = lq::taskId(task);
        const QString taskId = QString::fromLatin1(id.data(), static_cast<qsizetype>(id.size()));
        v.tasks.append(QVariantMap{
            {QStringLiteral("id"), taskId},
            {QStringLiteral("name"), text.name},
            {QStringLiteral("detail"), text.detail},
            {QStringLiteral("tone"), text.tone},
            {QStringLiteral("minutes"), lq::taskMinutes(task)},
            {QStringLiteral("done"), status.finished || lq::taskDone(state, status.n, task)},
            {QStringLiteral("route"), m_learner->routeForTask(taskId)},
        });
    }

    // Streak and this week.
    const lq::StreakInfo streak = lq::streakInfo(state, today);
    const std::set<lq::Date> active = lq::activeDays(state);
    v.streak = streak.count;
    v.longestStreak = streak.longest;
    v.freezes = streak.freezes;
    v.todayDone = streak.todayDone;
    static const QStringList weekLabels{QStringLiteral("Mon"), QStringLiteral("Tue"), QStringLiteral("Wed"),
                                        QStringLiteral("Thu"), QStringLiteral("Fri"), QStringLiteral("Sat"),
                                        QStringLiteral("Sun")};
    for (std::size_t i = 0; i < streak.week.size(); ++i) {
        const lq::Date day = streak.week[i].date;
        v.week.append(QVariantMap{
            {QStringLiteral("label"), weekLabels.at(static_cast<qsizetype>(i))},
            {QStringLiteral("day"), toQDate(day).day()},
            {QStringLiteral("state"), dayState(day, today, state, active)},
        });
    }

    // Pace against the calendar.
    const lq::Pace pace = lq::pace(state, today);
    v.doneCount = pace.doneCount;
    v.paceDiff = pace.diff;
    if (status.finished)
        v.paceText = QStringLiteral("Plan complete");
    else if (pace.diff == 0)
        v.paceText = QStringLiteral("On track");
    else
        v.paceText = days(std::abs(pace.diff)) + (pace.diff > 0 ? QStringLiteral(" ahead") : QStringLiteral(" behind"));
    if (pace.target)
        v.targetText = en.toString(toQDate(*pace.target), QStringLiteral("MMM d, yyyy"));

    for (const lq::Phase &phase : course.phases) {
        QString phaseState = QStringLiteral("upcoming");
        if (status.finished || state.checkpoints.contains(phase.n))
            phaseState = QStringLiteral("done");
        else if (phase.n == entry.phase)
            phaseState = QStringLiteral("current");
        v.phases.append(QVariantMap{
            {QStringLiteral("n"), phase.n},
            {QStringLiteral("title"), qs(phase.title)},
            {QStringLiteral("state"), phaseState},
        });
    }

    // Five weeks, Monday to Sunday, ending with the current one.
    const lq::Date first = lq::addDays(lq::weekStart(today), -28);
    for (int i = 0; i < 35; ++i) {
        const lq::Date day = lq::addDays(first, i);
        v.calendar.append(QVariantMap{
            {QStringLiteral("day"), toQDate(day).day()},
            {QStringLiteral("state"), dayState(day, today, state, active)},
        });
    }
    v.calendarTitle = en.toString(toQDate(first), QStringLiteral("MMM d")) + QStringLiteral(" – ")
                      + en.toString(toQDate(lq::addDays(first, 34)), QStringLiteral("MMM d"));

    v.coach = m_learner->coach({}).mid(0, kCoachTips);

    m_view = v;
    emit changed();
}
