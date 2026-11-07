#pragma once

#include "lq/course.h"
#include "lq/exercises.h"
#include "lq/progress.h"

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The coach. It suggests what to do next, each time with the reason in the learner's own numbers, and reads
// their reflections to learn what they enjoy, which tilts mixed practice towards it. The plan never changes:
// the coach orients, it does not decide.
namespace lq {

// What the reflections say, as a weight per exercise kind.
struct Taste {
    std::map<std::string, double, std::less<>> weights;  // kind → 0.5 (offered, never enjoyed) … 1.5 (always enjoyed)
    std::optional<Feel> feel;                            // how the latest session felt, when it was asked
    Date feltOn{};                                       // the day of that reflection
    int reflections = 0;                                 // how many reflections it rests on
    double weight(std::string_view kind) const;          // 1 for a kind with no opinion yet
    std::vector<std::string> favourites() const;         // kinds clearly enjoyed, most enjoyed first
};

inline constexpr std::size_t kTasteMemory = 6;  // only the latest reflections count, so taste can change
inline constexpr double kFavourite = 1.1;       // weight above which a kind counts as enjoyed

Taste taste(const LearnerState &state);
// How the reflection question and the coach name a kind: "listening" for "listen".
std::string_view kindLabel(std::string_view kind);
// One sentence on what the learner's taste changes, shown when they save a reflection.
std::string tasteSummary(const Taste &taste);

// n items from a larger pool, drawn with the learner's weights. After an easy session typed and built answers
// weigh more; after a hard one, choosing and matching do. Nothing is ruled out, only made less likely.
std::vector<Item> preferredItems(std::vector<Item> pool, std::size_t n, const Taste &taste, SeededRandom &rnd);

struct CoachTip {
    // Which rule: "review", "behind", "checkpoint", "skill", "lesson", "speak", "hard", "easy", "enjoy",
    // "ahead" or "plan".
    std::string id;
    std::string title;         // what to do
    std::string detail;        // why, with the learner's numbers
    std::string trainer;       // the trainer to open ("review", "verbs"…), if any
    int unit = -1;             // the lesson to practise again, if any
    std::optional<Task> task;  // the plan task to open, if any
};

inline constexpr int kReviewPile = 15;       // due cards that make review the first suggestion
inline constexpr int kSkillWindow = 14;      // days of answers the coach looks at
inline constexpr int kSkillMinAnswers = 8;   // fewer answers say nothing yet
inline constexpr double kWeakSkill = 0.7;
inline constexpr double kWeakLesson = 0.6;
inline constexpr int kQuietDays = 4;         // days without saying anything out loud before the coach mentions it
inline constexpr int kCheckpointAhead = 3;   // plan days before a checkpoint when the coach starts preparing for it
inline constexpr int kFeelDays = 7;          // how long an easy or hard session shapes the suggestions

// Suggestions, most useful first. `after` is the activity just finished ("lesson" or a trainer id): the coach
// does not send the learner straight back to it.
std::vector<CoachTip> coachTips(const LearnerState &state, const Course &course, Date today, std::string_view after = {});

} // namespace lq
