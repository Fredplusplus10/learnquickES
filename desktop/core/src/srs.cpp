#include "lq/srs.h"

#include <algorithm>
#include <tuple>

namespace lq {

Card newCard(Date today)
{
    Card card;
    card.due = addDays(today, 1);
    card.added = today;
    return card;
}

Card gradeCard(const Card &card, Grade grade, Date today)
{
    Card next = card;
    ++next.reps;
    next.last = today;
    if (grade == Grade::Again) {
        next.box = 1;
        ++next.lapses;
        next.due = addDays(today, 1);
        return next;
    }
    next.box = std::min(kMaxBox, card.box + (grade == Grade::Easy ? 2 : 1));
    next.due = addDays(today, kSrsIntervals[static_cast<std::size_t>(next.box)]);
    return next;
}

std::vector<std::string> reviewQueue(const Deck &deck, Date today, int limit)
{
    std::vector<std::pair<std::string, const Card *>> due;
    for (const auto &[id, card] : deck) {
        if (isDue(card, today))
            due.emplace_back(id, &card);
    }
    std::sort(due.begin(), due.end(), [](const auto &a, const auto &b) {
        return std::tie(a.second->due, a.second->box, a.second->seq) < std::tie(b.second->due, b.second->box, b.second->seq);
    });
    std::vector<std::string> ids;
    for (std::size_t i = 0; i < due.size() && static_cast<int>(i) < limit; ++i)
        ids.push_back(due[i].first);
    return ids;
}

} // namespace lq
