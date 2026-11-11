#include "lq/progress.h"

#include "lq/text.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace lq {

namespace {

IsScheduled scheduledFor(const Plan &plan)
{
    return [&plan](Date d) { return isStudyDay(d, plan); };
}

void append(Events &to, const Events &from)
{
    to.insert(to.end(), from.begin(), from.end());
}

const ScheduleEntry &entryAt(const LearnerState &state, int n)
{
    return schedule(state.plan.days)[static_cast<std::size_t>(n - 1)];
}

// At most `limit` characters, never cut inside one: UTF-8 continuation bytes are 10xxxxxx.
std::string trimmed(std::string_view text, std::size_t limit)
{
    constexpr std::string_view space = " \t\r\n";
    const auto first = text.find_first_not_of(space);
    if (first == std::string_view::npos)
        return {};
    const auto last = text.find_last_not_of(space);
    return cutText(text.substr(first, last - first + 1), limit);
}

} // namespace

LearnerState defaultState(Date today)
{
    LearnerState state;
    state.plan.start = today;
    return state;
}

const std::vector<ScheduleEntry> &schedule(int totalDays)
{
    static const std::map<int, std::vector<ScheduleEntry>> schedules{
        {30, buildSchedule(30)}, {60, buildSchedule(60)}, {90, buildSchedule(90)}};
    return schedules.at(totalDays);
}

std::set<Date> activeDays(const LearnerState &state)
{
    std::set<Date> active;
    for (const auto &[date, activity] : state.activity) {
        if (activity.xp >= kMinXp)
            active.insert(date);
    }
    return active;
}

int streakCount(const LearnerState &state, Date today)
{
    return currentStreak(today, activeDays(state), state.frozen, scheduledFor(state.plan));
}

Events addXp(LearnerState &state, int xp, Date today)
{
    Activity &activity = state.activity[today];
    const int before = activity.xp;
    activity.xp += xp;
    Events events;
    // Crossing kMinXp for the first time today extends the streak (Duolingo-style, separate from the plan).
    if (before < kMinXp && activity.xp >= kMinXp) {
        const int count = streakCount(state, today);
        const int gain = freezesEarned(count, isStudyDay(today, state.plan), state.freezes);
        if (gain > 0) {
            state.freezes += gain;
            events.push_back(Event{EventType::FreezeEarned, gain});
        }
        state.longest = std::max(state.longest, count);
        events.push_back(Event{EventType::Streak, count});
    }
    return events;
}

Events applyFreezes(LearnerState &state, Date today)
{
    const FreezeResult result = freezesNeeded(today, activeDays(state), state.frozen, scheduledFor(state.plan), state.freezes);
    if (result.used.empty())
        return {};
    state.frozen.insert(result.used.begin(), result.used.end());
    state.freezes = result.freezes;
    return {Event{EventType::FreezeUsed, static_cast<int>(result.used.size()), result.used}};
}

StreakInfo streakInfo(const LearnerState &state, Date today)
{
    const std::set<Date> active = activeDays(state);
    const IsScheduled isScheduled = scheduledFor(state.plan);
    StreakInfo info{.count = currentStreak(today, active, state.frozen, isScheduled),
                    .freezes = state.freezes,
                    .todayDone = active.contains(today)};
    const Date monday = weekStart(today);
    for (std::size_t i = 0; i < info.week.size(); ++i) {
        const Date d = addDays(monday, static_cast<int>(i));
        DayState dayState = DayState::Off;
        if (active.contains(d))
            dayState = DayState::Done;
        else if (state.frozen.contains(d))
            dayState = DayState::Frozen;
        else if (d > today)
            dayState = DayState::Future;
        else if (d == today)
            dayState = DayState::Today;
        else if (isScheduled(d))
            dayState = DayState::Missed;
        info.week[i] = {d, dayState, isScheduled(d)};
    }
    info.longest = std::max(state.longest, longestStreak(active, state.frozen, isScheduled));
    return info;
}

PlanStatus planStatus(const LearnerState &state)
{
    PlanStatus status;
    status.total = state.plan.days;
    status.doneCount = static_cast<int>(state.done.size());
    status.finished = status.doneCount >= status.total;
    status.n = std::min(status.doneCount + 1, status.total);
    status.entry = entryAt(state, status.n);
    status.tasks = dayTasks(status.entry, status.total);
    status.minutes = dayMinutes(status.tasks);
    return status;
}

bool taskDone(const LearnerState &state, int n, Task task)
{
    const ScheduleEntry &entry = entryAt(state, n);
    if (task == Task::Lesson)
        return state.units.contains(entry.unit);
    if (task == Task::Checkpoint || task == Task::Final)
        return state.checkpoints.contains(entry.phase);
    const auto day = state.tasks.find(n);
    return day != state.tasks.end() && day->second.contains(task);
}

Events markTask(LearnerState &state, Task task, Date today)
{
    const PlanStatus status = planStatus(state);
    if (status.finished || std::find(status.tasks.begin(), status.tasks.end(), task) == status.tasks.end())
        return {};
    state.tasks[status.n][task] = today;
    return refreshDay(state, today);
}

Events refreshDay(LearnerState &state, Date today)
{
    const PlanStatus status = planStatus(state);
    if (status.finished)
        return {};
    for (Task task : status.tasks) {
        if (!taskDone(state, status.n, task))
            return {};
    }
    state.done.push_back({status.n, today});
    Events events{Event{EventType::DayComplete, status.n}};
    append(events, addXp(state, kXpDayBonus, today));
    if (status.entry.type == DayType::Checkpoint)
        events.push_back(Event{EventType::PhaseComplete, status.entry.phase});
    return events;
}

Events learnUnit(LearnerState &state, int unit, const std::vector<std::string> &words, double score, Date today)
{
    Score next{today, score, score, 1};
    if (const auto prev = state.units.find(unit); prev != state.units.end())
        next = {prev->second.date, score, std::max(score, prev->second.best), prev->second.attempts + 1};
    state.units[unit] = next;
    for (const std::string &id : words) {
        if (state.cards.contains(id))
            continue;
        Card card = newCard(today);
        card.unit = unit;
        card.seq = static_cast<int>(state.cards.size());
        state.cards.emplace(id, card);
    }
    Events events = addXp(state, kXpLessonBonus, today);
    append(events, refreshDay(state, today));
    return events;
}

Events recordAnswer(LearnerState &state, std::string_view skill, bool ok, bool almost, bool retry, Date today)
{
    auto &answers = state.activity[today].answers;
    auto tally = answers.find(skill);
    if (tally == answers.end())
        tally = answers.emplace(std::string(skill), Tally{}).first;
    // A requeued correction is practice, not another first attempt for accuracy.
    if (!retry) {
        ++tally->second.total;
        if (ok)
            ++tally->second.correct;
    }
    return addXp(state, !ok ? 0 : almost ? kXpAlmost : retry ? kXpRetry : kXpCorrect, today);
}

Events recordPhrase(LearnerState &state, Date today)
{
    ++state.activity[today].speak;
    return addXp(state, kXpPhrase, today);
}

Events gradeReview(LearnerState &state, const std::string &id, int unit, Grade grade, Date today)
{
    Card card;
    if (const auto it = state.cards.find(id); it != state.cards.end()) {
        card = it->second;
    } else {
        card = newCard(today);
        card.unit = unit;
        card.seq = static_cast<int>(state.cards.size());
    }
    state.cards[id] = gradeCard(card, grade, today);
    Tally &tally = state.activity[today].answers["review"];
    ++tally.total;
    if (grade != Grade::Again)
        ++tally.correct;
    return addXp(state, grade != Grade::Again ? kXpCard : 1, today);
}

ReviewSync syncReview(LearnerState &state, Date today)
{
    ReviewSync result;
    const PlanStatus status = planStatus(state);
    if (status.finished || std::find(status.tasks.begin(), status.tasks.end(), Task::Review) == status.tasks.end())
        return result;
    const bool due = !reviewQueue(state.cards, today, 1).empty();
    const auto day = state.tasks.find(status.n);
    const bool marked = day != state.tasks.end() && day->second.contains(Task::Review);
    if (marked && day->second.at(Task::Review) < today && due) {
        day->second.erase(Task::Review);
        result.changed = true;
    } else if (!marked && !due) {
        result.events = markTask(state, Task::Review, today);
        result.changed = true;
    }
    return result;
}

Events saveCheckpoint(LearnerState &state, int phase, double score, Date today)
{
    Score next{today, score, score, 1};
    if (const auto prev = state.checkpoints.find(phase); prev != state.checkpoints.end())
        next = {prev->second.date, score, std::max(score, prev->second.best), prev->second.attempts + 1};
    state.checkpoints[phase] = next;
    return refreshDay(state, today);
}

Pace pace(const LearnerState &state, Date today)
{
    const Plan &plan = state.plan;
    const Date yesterday = addDays(today, -1);
    Pace result;
    result.doneCount = static_cast<int>(state.done.size());
    result.expected = std::min(plan.days, plan.start <= yesterday ? countStudyDays(plan.start, yesterday, plan) : 0);
    result.diff = result.doneCount - result.expected;
    result.remaining = plan.days - result.doneCount;
    const bool doneToday = std::any_of(state.done.begin(), state.done.end(), [today](const DoneDay &d) { return d.date == today; });
    const Date from = isStudyDay(today, plan) && !doneToday ? today : addDays(today, 1);
    if (result.remaining > 0)
        result.target = nthStudyDay(from, result.remaining, plan);
    else if (!state.done.empty())
        result.target = state.done.back().date;
    return result;
}

int recommendedPlanDays(std::string_view level)
{
    if (level == "A1")
        return 90;
    if (level == "A2+")
        return 30;
    return 60;
}

std::string seriesLevel(std::string_view startLevel, const LearnerState &state)
{
    const auto passed = [&state](int phase) {
        const auto found = state.checkpoints.find(phase);
        return found != state.checkpoints.end() && found->second.best >= kCheckpointPass;
    };
    if (passed(kSeriesB1Phase))
        return "B1";
    if (startLevel == "A2" || startLevel == "A2+" || passed(kSeriesA2Phase))
        return "A2";
    return "A1";
}

void countSession(LearnerState &state)
{
    ++state.sessions;
}

bool reflectionDue(const LearnerState &state)
{
    return state.sessions - state.reflectedAt >= kReflectEvery;
}

void addReflection(LearnerState &state, Reflection reflection)
{
    reflection.note = cutText(std::move(reflection.note), kMaxNoteLength);
    state.reflections.push_back(std::move(reflection));
    state.reflectedAt = state.sessions;
}

void postponeReflection(LearnerState &state)
{
    state.reflectedAt = state.sessions;
}

std::string_view feelId(Feel feel)
{
    switch (feel) {
    case Feel::Easy: return "easy";
    case Feel::Right: return "right";
    case Feel::Hard: return "hard";
    }
    return {};
}

std::optional<Feel> feelFromId(std::string_view id)
{
    for (const Feel feel : {Feel::Easy, Feel::Right, Feel::Hard}) {
        if (feelId(feel) == id)
            return feel;
    }
    return std::nullopt;
}

const std::vector<std::string_view> &inputKinds()
{
    static const std::vector<std::string_view> kinds{"podcast", "video", "series", "reading", "music", "other", "island"};
    return kinds;
}

Events logInput(LearnerState &state, int minutes, std::string_view kind, std::string_view source, Date today)
{
    minutes = std::clamp(minutes, 1, kMaxLoggedMinutes);
    const auto &kinds = inputKinds();
    const std::string_view known = std::ranges::find(kinds, kind) != kinds.end() ? kind : std::string_view("other");
    Activity &activity = state.activity[today];
    (known == "island" ? activity.islandMinutes : activity.inputMinutes) += minutes;
    state.input.push_back({today, minutes, std::string(known), trimmed(source, kMaxLogText)});
    return addXp(state, minutes, today);
}

bool removeInput(LearnerState &state, std::size_t index)
{
    if (index >= state.input.size())
        return false;
    const InputEntry entry = state.input[index];
    state.input.erase(state.input.begin() + static_cast<std::ptrdiff_t>(index));
    if (const auto day = state.activity.find(entry.date); day != state.activity.end()) {
        int &minutes = entry.kind == "island" ? day->second.islandMinutes : day->second.inputMinutes;
        minutes = std::max(0, minutes - entry.minutes);
        if (!entry.imported)
            day->second.xp = std::max(0, day->second.xp - entry.minutes);
    }
    return true;
}

Events logExchange(LearnerState &state, int minutes, std::string_view partner, std::string_view note, Date today)
{
    minutes = std::clamp(minutes, 1, kMaxExchangeMinutes);
    state.activity[today].talkMinutes += minutes;
    state.exchanges.push_back({today, minutes, trimmed(partner, kMaxLogText), trimmed(note, kMaxLogText)});
    return addXp(state, minutes, today);
}

void setMission(LearnerState &state, std::string_view id, bool done, Date today)
{
    if (!done) {
        if (const auto it = state.missions.find(id); it != state.missions.end())
            state.missions.erase(it);
    } else if (!state.missions.contains(id)) {
        state.missions.emplace(std::string(id), today);
    }
}

ListeningLevel listeningLevel(int inputMinutes)
{
    const double hours = inputMinutes / 60.0;
    ListeningLevel level;
    level.level = static_cast<int>(std::ranges::count_if(kListeningMilestones, [hours](int h) { return hours >= h; }));
    if (level.level < static_cast<int>(kListeningMilestones.size())) {
        const int from = kListeningMilestones[static_cast<std::size_t>(level.level - 1)];
        level.nextHours = kListeningMilestones[static_cast<std::size_t>(level.level)];
        level.progress = (hours - from) / (level.nextHours - from);
    } else {
        level.progress = 1.0;
    }
    return level;
}

int Totals::answered(std::string_view skill) const
{
    const auto it = answers.find(skill);
    return it == answers.end() ? 0 : it->second.total;
}

std::optional<double> Totals::accuracy(std::string_view skill) const
{
    const auto it = answers.find(skill);
    if (it == answers.end() || it->second.total == 0)
        return std::nullopt;
    return static_cast<double>(it->second.correct) / it->second.total;
}

Totals totals(const LearnerState &state, Date today)
{
    Totals t;
    for (const auto &[day, a] : state.activity) {
        t.xp += a.xp;
        t.appMinutes += a.seconds / 60.0;
        t.inputMinutes += a.inputMinutes;
        t.islandMinutes += a.islandMinutes;
        t.talkMinutes += a.talkMinutes;
        t.speak += a.speak;
        for (const auto &[skill, tally] : a.answers) {
            Tally &sum = t.answers[skill];
            sum.correct += tally.correct;
            sum.total += tally.total;
        }
    }
    for (const auto &[id, card] : state.cards) {
        ++t.wordsSeen;
        t.wordsRemembered += remembered(card) ? 1 : 0;
        t.wordsDue += isDue(card, today) ? 1 : 0;
    }
    return t;
}

int spanishMinutes(const LearnerState &state, Date day)
{
    const auto it = state.activity.find(day);
    if (it == state.activity.end())
        return 0;
    const Activity &a = it->second;
    return static_cast<int>(std::lround(a.seconds / 60.0)) + a.inputMinutes + a.islandMinutes + a.talkMinutes;
}

} // namespace lq
