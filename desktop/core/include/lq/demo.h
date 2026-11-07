#pragma once

#include "lq/progress.h"
#include "lq/random.h"

#include <string>
#include <vector>

namespace lq {

// Sample learner about three weeks into a 60-day plan, built through the real domain functions.
// unitWords: the vocabulary ids of each unit (Course::unitWordIds), so finished lessons fill the deck.
LearnerState demoState(Date today, const std::vector<std::vector<std::string>> &unitWords);

} // namespace lq
