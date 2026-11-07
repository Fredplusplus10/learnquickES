#include "lq/calendar.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <algorithm>
#include <vector>

using namespace lq;
using lq::test::d;

TEST_CASE("US federal holidays use the observed rule", "[calendar]")
{
    const std::set<Date> expected{d("2026-01-01"), d("2026-01-19"), d("2026-02-16"), d("2026-05-25"),
                                  d("2026-06-19"), d("2026-07-03"), d("2026-09-07"), d("2026-10-12"),
                                  d("2026-11-11"), d("2026-11-26"), d("2026-12-25")};
    CHECK(usFederalHolidays(2026) == expected);
    // New Year 2022 fell on a Saturday → observed Friday Dec 31, 2021.
    CHECK(isHoliday(d("2021-12-31")));
    CHECK_FALSE(isHoliday(d("2026-07-04")));
}

TEST_CASE("study days skip weekends, holidays and days off", "[calendar]")
{
    Plan plan;
    plan.daysOff = {d("2026-11-04")};
    CHECK(isStudyDay(d("2026-11-02"), plan));
    CHECK_FALSE(isStudyDay(d("2026-11-04"), plan));  // day off
    CHECK_FALSE(isStudyDay(d("2026-11-07"), plan));  // Saturday
    CHECK_FALSE(isStudyDay(d("2026-11-11"), plan));  // Veterans Day
    Plan worksHolidays = plan;
    worksHolidays.skipHolidays = false;
    CHECK(isStudyDay(d("2026-11-11"), worksHolidays));
    CHECK(nthStudyDay(d("2026-11-02"), 5, plan) == d("2026-11-09"));
    CHECK(countStudyDays(d("2026-11-02"), d("2026-11-13"), plan) == 8);
}

TEST_CASE("a plan without study days has no n-th study day", "[calendar]")
{
    Plan plan;
    plan.studyDays.clear();
    CHECK_FALSE(nthStudyDay(d("2026-11-02"), 1, plan));
    CHECK_FALSE(nthStudyDay(d("2026-11-02"), 0, Plan{}));
}

TEST_CASE("every schedule covers all 25 units in order with 5 checkpoints", "[calendar]")
{
    const int days = GENERATE(30, 60, 90);
    CAPTURE(days);
    const auto schedule = buildSchedule(days);
    REQUIRE(schedule.size() == static_cast<std::size_t>(days));

    std::vector<int> units, checkpoints;
    for (const auto &entry : schedule) {
        if (entry.type == DayType::Lesson) {
            units.push_back(entry.unit);
            CHECK(entry.phase == entry.unit / kUnitsPerPhase + 1);
        } else if (entry.type == DayType::Checkpoint) {
            checkpoints.push_back(entry.n);
        }
        CHECK(dayMinutes(dayTasks(entry, days)) <= 50);
    }
    std::vector<int> allUnits(kPhases * kUnitsPerPhase);
    for (int i = 0; i < static_cast<int>(allUnits.size()); ++i)
        allUnits[static_cast<std::size_t>(i)] = i;
    CHECK(units == allUnits);
    CHECK(checkpoints == std::vector<int>{days / 5, 2 * days / 5, 3 * days / 5, 4 * days / 5, days});
    CHECK(dayTasks(schedule.back(), days) == std::vector<Task>{Task::Review, Task::Final});
}

TEST_CASE("only the 30-day plan adds a focus trainer to lesson days", "[calendar]")
{
    const auto first30 = buildSchedule(30).front();
    const auto first60 = buildSchedule(60).front();
    REQUIRE(first30.type == DayType::Lesson);
    REQUIRE(first60.type == DayType::Lesson);
    CHECK(dayTasks(first30, 30) == std::vector<Task>{Task::Review, Task::Lesson, Task::Verbs, Task::Cognates});
    CHECK(dayTasks(first60, 60) == std::vector<Task>{Task::Review, Task::Lesson, Task::Verbs});
    CHECK(inputTargetMinutes(30) == 90);
    CHECK(inputTargetMinutes(60) == 60);
    CHECK(inputTargetMinutes(90) == 45);
}
