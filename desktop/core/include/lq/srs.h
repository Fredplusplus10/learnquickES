#pragma once

#include "lq/date.h"

#include <array>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

// Spaced repetition, ported from app/js/core/srs.js: a transparent Leitner-style scheduler. In a 30–90 day
// course each card is seen a handful of times, too few reviews to fit FSRS parameters, so fixed expanding
// intervals are the honest choice.
namespace lq {

inline constexpr std::array<int, 8> kSrsIntervals{0, 1, 2, 4, 8, 16, 32, 60};  // days, by box
inline constexpr int kMaxBox = static_cast<int>(kSrsIntervals.size()) - 1;
// Recalled after 1, 2 and 4 days: the next gap is 8 days.
inline constexpr int kRememberedBox = 4;
inline constexpr int kDefaultReviewLimit = 20;

enum class Grade { Again = 0, Good = 2, Easy = 3 };

struct Card {
    int unit = -1;  // the lesson that added it (0..24)
    int box = 1;
    Date due{};
    int reps = 0;
    int lapses = 0;
    Date added{};
    std::optional<Date> last;
    int seq = 0;  // position in the deck, which breaks ties in the review queue
    bool operator==(const Card &) const = default;
};

using Deck = std::map<std::string, Card, std::less<>>;  // vocabulary id → card

Card newCard(Date today);
Card gradeCard(const Card &card, Grade grade, Date today);
inline bool isDue(const Card &card, Date today) { return card.due <= today; }
inline bool remembered(const Card &card) { return card.box >= kRememberedBox; }
// Due cards, most overdue first; ties go to the weakest box, then to the oldest card.
std::vector<std::string> reviewQueue(const Deck &deck, Date today, int limit = kDefaultReviewLimit);

} // namespace lq
