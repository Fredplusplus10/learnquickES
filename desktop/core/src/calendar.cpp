#include "lq/calendar.h"

#include <algorithm>
#include <array>
#include <numeric>

namespace lq {

using namespace std::chrono;

namespace {

// Fixed-date holidays move to Friday/Monday when they fall on a weekend (federal "observed" rule).
Date observed(Date date)
{
    switch (weekdayOf(date)) {
    case 6:
        return addDays(date, -1);
    case 0:
        return addDays(date, 1);
    default:
        return date;
    }
}

Date nthWeekday(int y, month m, weekday wd, unsigned n)
{
    return sys_days{year{y} / m / wd[n]};
}

Date lastWeekday(int y, month m, weekday wd)
{
    return sys_days{year{y} / m / wd[std::chrono::last]};
}

// Focus trainers rotate per phase so practice days stay varied but follow the course.
constexpr std::array<std::array<Task, 3>, kPhases> kFocus{{
    {Task::Cognates, Task::Gender, Task::Speak},
    {Task::Speak, Task::Gender, Task::Cognates},
    {Task::Past, Task::Speak, Task::Past},
    {Task::Past, Task::Speak, Task::FalseFriends},
    {Task::Past, Task::Speak, Task::Describe},
}};

} // namespace

std::set<Date> usFederalHolidays(int y)
{
    return {
        observed(makeDate(y, 1, 1)),        // New Year's Day
        nthWeekday(y, January, Monday, 3),  // Martin Luther King Jr. Day
        nthWeekday(y, February, Monday, 3), // Washington's Birthday
        lastWeekday(y, May, Monday),        // Memorial Day
        observed(makeDate(y, 6, 19)),       // Juneteenth
        observed(makeDate(y, 7, 4)),        // Independence Day
        nthWeekday(y, September, Monday, 1), // Labor Day
        nthWeekday(y, October, Monday, 2),  // Columbus Day
        observed(makeDate(y, 11, 11)),      // Veterans Day
        nthWeekday(y, November, Thursday, 4), // Thanksgiving
        observed(makeDate(y, 12, 25)),      // Christmas Day
    };
}

bool isHoliday(Date date)
{
    // Next year's New Year's Day can be observed on December 31.
    const int y = yearOf(date);
    return usFederalHolidays(y).contains(date) || usFederalHolidays(y + 1).contains(date);
}

bool isStudyDay(Date date, const Plan &plan)
{
    if (!plan.studyDays.contains(weekdayOf(date)))
        return false;
    if (plan.skipHolidays && isHoliday(date))
        return false;
    return !plan.daysOff.contains(date);
}

int countStudyDays(Date from, Date to, const Plan &plan)
{
    int n = 0;
    for (Date d = from; d <= to; d = addDays(d, 1))
        n += isStudyDay(d, plan) ? 1 : 0;
    return n;
}

std::optional<Date> nthStudyDay(Date from, int k, const Plan &plan)
{
    if (k <= 0)
        return std::nullopt;
    // Ten years without k study days means the plan has none (e.g. every weekday is off).
    for (Date d = from; d < addDays(from, 3660); d = addDays(d, 1)) {
        if (isStudyDay(d, plan) && --k == 0)
            return d;
    }
    return std::nullopt;
}

std::vector<ScheduleEntry> buildSchedule(int totalDays)
{
    const int per = totalDays / kPhases;
    std::vector<ScheduleEntry> out;
    out.reserve(static_cast<std::size_t>(totalDays));
    for (int p = 0; p < kPhases; ++p) {
        std::array<int, kUnitsPerPhase> lessonAt{};
        for (int i = 0; i < kUnitsPerPhase; ++i)
            lessonAt[static_cast<std::size_t>(i)] = i * (per - 1) / kUnitsPerPhase;
        int practice = 0;
        for (int j = 0; j < per; ++j) {
            ScheduleEntry entry{.n = p * per + j + 1, .phase = p + 1};
            const auto lesson = std::find(lessonAt.begin(), lessonAt.end(), j);
            if (j == per - 1) {
                entry.type = DayType::Checkpoint;
            } else if (lesson != lessonAt.end()) {
                entry.type = DayType::Lesson;
                entry.unit = p * kUnitsPerPhase + static_cast<int>(lesson - lessonAt.begin());
            } else {
                entry.type = DayType::Practice;
                entry.practiceIndex = practice++;
            }
            out.push_back(entry);
        }
    }
    return out;
}

std::vector<Task> dayTasks(const ScheduleEntry &entry, int totalDays)
{
    const auto &focus = kFocus[static_cast<std::size_t>(entry.phase - 1)];
    switch (entry.type) {
    case DayType::Checkpoint:
        return {Task::Review, entry.phase == kPhases ? Task::Final : Task::Checkpoint};
    case DayType::Practice:
        return {Task::Review, Task::Mixed, Task::Verbs, focus[static_cast<std::size_t>(entry.practiceIndex % 3)]};
    case DayType::Lesson: {
        std::vector<Task> tasks{Task::Review, Task::Lesson, Task::Verbs};
        // The 30-day plan has no practice days, so each lesson day carries one focus trainer too.
        if (totalDays == 30)
            tasks.push_back(focus[static_cast<std::size_t>(entry.unit % 3)]);
        return tasks;
    }
    }
    return {};
}

int taskMinutes(Task task)
{
    switch (task) {
    case Task::Review: return 5;
    case Task::Lesson: return 20;
    case Task::Verbs: return 5;
    case Task::Mixed: return 8;
    case Task::Speak: return 6;
    case Task::Cognates: return 6;
    case Task::Gender: return 5;
    case Task::Past: return 8;
    case Task::FalseFriends: return 5;
    case Task::Describe: return 8;
    case Task::Checkpoint: return 15;
    case Task::Final: return 45;
    }
    return 5;
}

int dayMinutes(const std::vector<Task> &tasks)
{
    return std::accumulate(tasks.begin(), tasks.end(), 0, [](int sum, Task t) { return sum + taskMinutes(t); });
}

std::string_view taskId(Task task)
{
    switch (task) {
    case Task::Review: return "review";
    case Task::Lesson: return "lesson";
    case Task::Verbs: return "verbs";
    case Task::Mixed: return "mixed";
    case Task::Speak: return "speak";
    case Task::Cognates: return "cognates";
    case Task::Gender: return "gender";
    case Task::Past: return "past";
    case Task::FalseFriends: return "falsefriends";
    case Task::Describe: return "describe";
    case Task::Checkpoint: return "checkpoint";
    case Task::Final: return "final";
    }
    return "";
}

std::optional<Task> taskFromId(std::string_view id)
{
    for (int i = 0; i <= static_cast<int>(Task::Final); ++i) {
        const auto task = static_cast<Task>(i);
        if (taskId(task) == id)
            return task;
    }
    return std::nullopt;
}

int inputTargetMinutes(int totalDays)
{
    switch (totalDays) {
    case 30: return 90;
    case 90: return 45;
    default: return 60;
    }
}

} // namespace lq
