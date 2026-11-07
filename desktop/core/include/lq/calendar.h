#pragma once

#include "lq/date.h"

#include <optional>
#include <set>
#include <string_view>
#include <vector>

namespace lq {

inline constexpr int kPhases = 5;
inline constexpr int kUnitsPerPhase = 5;

// Which calendar days are study days. Mirrors plan.studyDays / skipHolidays / daysOff in the web app.
struct Plan {
    int days = 60;                           // 30, 60 or 90 study days
    Date start{};
    std::set<int> studyDays{1, 2, 3, 4, 5};  // weekdays, 0 = Sunday
    bool skipHolidays = true;                // US federal holidays
    std::set<Date> daysOff;
    bool operator==(const Plan &) const = default;
};

std::set<Date> usFederalHolidays(int year);  // observed dates
bool isHoliday(Date date);
bool isStudyDay(Date date, const Plan &plan);
int countStudyDays(Date from, Date to, const Plan &plan);            // both ends included
std::optional<Date> nthStudyDay(Date from, int k, const Plan &plan);  // k-th study day from `from`, included

enum class DayType { Lesson, Practice, Checkpoint };

struct ScheduleEntry {
    int n = 0;              // plan day, 1-based
    int phase = 0;          // 1..5
    DayType type = DayType::Lesson;
    int unit = -1;          // lesson days: 0..24
    int practiceIndex = -1; // practice days: position among the phase's practice days
};

// Same 25 units for every plan length; longer plans interleave practice days.
// The last day of each phase is a checkpoint.
std::vector<ScheduleEntry> buildSchedule(int totalDays);

enum class Task { Review, Lesson, Verbs, Mixed, Speak, Cognates, Gender, Past, FalseFriends, Describe, Checkpoint, Final };

std::vector<Task> dayTasks(const ScheduleEntry &entry, int totalDays);
int taskMinutes(Task task);
int dayMinutes(const std::vector<Task> &tasks);
std::string_view taskId(Task task);     // "review", "lesson", … as in the web app
std::optional<Task> taskFromId(std::string_view id);
int inputTargetMinutes(int totalDays);  // daily Spanish listening outside the app

} // namespace lq
