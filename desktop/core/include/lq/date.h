#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

namespace lq {

// A study day is a date in the learner's own calendar, not a point in time. sys_days is a plain
// count of days, so there are no time zones or daylight-saving shifts to get wrong.
using Date = std::chrono::sys_days;

Date makeDate(int year, unsigned month, unsigned day);
std::optional<Date> parseIso(std::string_view text);   // "YYYY-MM-DD"
std::string toIso(Date date);

inline Date addDays(Date date, int n) { return date + std::chrono::days{n}; }
inline int diffDays(Date from, Date to) { return static_cast<int>((to - from).count()); }

// 0 = Sunday … 6 = Saturday, the same numbering as JavaScript's Date.getDay().
int weekdayOf(Date date);
// Monday of the week that contains date.
Date weekStart(Date date);
int yearOf(Date date);

} // namespace lq
