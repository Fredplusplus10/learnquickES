#pragma once

#include "lq/progress.h"

#include <string>
#include <string_view>
#include <vector>

// Achievements and B1 readiness signals, ported from app/js/core/progress.js and app/js/views/progress.js.
// Every one is earned from recorded activity, never shown as a sample.
namespace lq {

struct Achievement {
    std::string_view id;
    std::string_view icon;  // a line icon of the app's set
    std::string_view title;
    std::string_view how;   // what earns it
};

const std::vector<Achievement> &achievementList();

// Records every achievement the learner now qualifies for and has not earned yet, dated today; returns them
// in list order. Earned achievements stay earned.
std::vector<Achievement> checkAchievements(LearnerState &state, Date today);

// A concrete sign of being close to a functional B1: a guide, not a certificate.
struct ReadinessSignal {
    bool met = false;
    std::string text;  // with the learner's current number, e.g. "200+ words remembered (37)"
};

std::vector<ReadinessSignal> readiness(const LearnerState &state, const Totals &totals);

} // namespace lq
