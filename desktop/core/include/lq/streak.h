#pragma once

#include "lq/date.h"

#include <functional>
#include <set>
#include <vector>

namespace lq {

// A day counts once the learner earns kMinXp. Non-study days (weekends, holidays, days off) never
// break the streak; studying on them still adds a day. A missed study day consumes a freeze if one is held.
inline constexpr int kMinXp = 10;
inline constexpr int kMaxFreezes = 2;
inline constexpr int kFreezeEvery = 5;

using IsScheduled = std::function<bool(Date)>;

// Today without activity yet does not break anything.
int currentStreak(Date today, const std::set<Date> &active, const std::set<Date> &frozen, const IsScheduled &isScheduled);

struct FreezeResult {
    std::vector<Date> used;  // missed study days now covered by a freeze
    int freezes = 0;         // freezes left
    bool broken = false;     // ran out of freezes
};

// Missed study days between the last active day and today, protected while freezes remain.
FreezeResult freezesNeeded(Date today, const std::set<Date> &active, const std::set<Date> &frozen,
                           const IsScheduled &isScheduled, int freezes);

// Freezes earned when today's activity first reaches kMinXp: every 5th streak day, and any study on a day off.
int freezesEarned(int streakCount, bool todayScheduled, int freezes);

// Longest run, with the same bridging rules.
int longestStreak(const std::set<Date> &active, const std::set<Date> &frozen, const IsScheduled &isScheduled);

} // namespace lq
