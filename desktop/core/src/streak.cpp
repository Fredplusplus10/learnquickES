#include "lq/streak.h"

#include <algorithm>

namespace lq {

int currentStreak(Date today, const std::set<Date> &active, const std::set<Date> &frozen, const IsScheduled &isScheduled)
{
    if (active.empty())
        return 0;
    const Date first = *active.begin();
    int count = 0;
    for (Date d = active.contains(today) ? today : addDays(today, -1); d >= first; d = addDays(d, -1)) {
        if (active.contains(d))
            ++count;
        else if (!frozen.contains(d) && isScheduled(d))
            break;
    }
    return count;
}

FreezeResult freezesNeeded(Date today, const std::set<Date> &active, const std::set<Date> &frozen,
                           const IsScheduled &isScheduled, int freezes)
{
    FreezeResult result{.freezes = freezes};
    if (active.empty())
        return result;
    for (Date d = addDays(*active.rbegin(), 1); d < today; d = addDays(d, 1)) {
        if (active.contains(d) || frozen.contains(d) || !isScheduled(d))
            continue;
        if (result.freezes == 0) {
            result.broken = true;
            break;
        }
        --result.freezes;
        result.used.push_back(d);
    }
    return result;
}

int freezesEarned(int streakCount, bool todayScheduled, int freezes)
{
    int gain = 0;
    if (streakCount > 0 && streakCount % kFreezeEvery == 0)
        ++gain;
    if (!todayScheduled)
        ++gain;
    return std::min(kMaxFreezes, freezes + gain) - freezes;
}

int longestStreak(const std::set<Date> &active, const std::set<Date> &frozen, const IsScheduled &isScheduled)
{
    if (active.empty())
        return 0;
    int best = 0, run = 0;
    for (Date d = *active.begin(); d <= *active.rbegin(); d = addDays(d, 1)) {
        if (active.contains(d))
            best = std::max(best, ++run);
        else if (!frozen.contains(d) && isScheduled(d))
            run = 0;
    }
    return best;
}

} // namespace lq
