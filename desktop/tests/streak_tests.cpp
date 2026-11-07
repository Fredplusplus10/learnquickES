#include "lq/calendar.h"
#include "lq/streak.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

using namespace lq;
using lq::test::d;

TEST_CASE("streak bridges weekends and non-study days, breaks on a missed study day", "[streak]")
{
    const Plan plan;
    const IsScheduled scheduled = [&plan](Date date) { return isStudyDay(date, plan); };
    const std::set<Date> active{d("2026-11-05"), d("2026-11-06"), d("2026-11-09")};  // Thu, Fri, (weekend), Mon

    CHECK(currentStreak(d("2026-11-09"), active, {}, scheduled) == 3);
    // Today without activity does not break it.
    CHECK(currentStreak(d("2026-11-10"), active, {}, scheduled) == 3);
    // Tuesday missed → broken (Wednesday 12; Veterans Day on the 11th is a holiday).
    CHECK(currentStreak(d("2026-11-12"), active, {}, scheduled) == 0);

    const FreezeResult result = freezesNeeded(d("2026-11-12"), active, {}, scheduled, 1);
    CHECK(result.used == std::vector<Date>{d("2026-11-10")});
    CHECK(result.freezes == 0);
    CHECK_FALSE(result.broken);
    const std::set<Date> frozen(result.used.begin(), result.used.end());
    CHECK(currentStreak(d("2026-11-12"), active, frozen, scheduled) == 3);

    CHECK(freezesNeeded(d("2026-11-13"), active, {}, scheduled, 1).broken);
    CHECK(longestStreak(active, {}, scheduled) == 3);
}

TEST_CASE("freezes are earned every 5 days and on days off, up to 2", "[streak]")
{
    CHECK(freezesEarned(5, true, 0) == 1);
    CHECK(freezesEarned(3, false, 1) == 1);  // studying on a day off
    CHECK(freezesEarned(10, false, 2) == 0); // capped
    CHECK(freezesEarned(10, false, 0) == 2);
    CHECK(freezesEarned(0, true, 0) == 0);
}
