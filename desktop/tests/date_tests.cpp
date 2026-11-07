#include "test_support.h"

#include <catch2/catch_test_macros.hpp>

using namespace lq;
using lq::test::d;

TEST_CASE("dates are calendar days and survive DST changes", "[date]")
{
    CHECK(addDays(d("2026-11-01"), 1) == d("2026-11-02"));  // US DST ends Nov 1, 2026
    CHECK(diffDays(d("2026-03-07"), d("2026-03-09")) == 2);  // DST starts Mar 8, 2026
    CHECK(weekStart(d("2026-11-08")) == d("2026-11-02"));    // Sunday → Monday of that week
    CHECK(weekStart(d("2026-11-02")) == d("2026-11-02"));
    CHECK(weekdayOf(d("2026-11-01")) == 0);
    CHECK(weekdayOf(d("2026-11-07")) == 6);
    CHECK(yearOf(d("2027-01-01")) == 2027);
}

TEST_CASE("ISO dates round-trip and reject malformed input", "[date]")
{
    CHECK(toIso(makeDate(2026, 1, 5)) == "2026-01-05");
    CHECK(parseIso("2028-02-29") == makeDate(2028, 2, 29));
    CHECK_FALSE(parseIso("2027-02-29"));  // not a leap year
    CHECK_FALSE(parseIso("2026-13-01"));
    CHECK_FALSE(parseIso("2026-1-05"));
    CHECK_FALSE(parseIso("2026-01-05T00"));
    CHECK_FALSE(parseIso("abcd-ef-gh"));
}
