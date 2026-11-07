#include "lq/demo.h"

#include "lq/achievements.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace lq {

LearnerState demoState(Date today, const std::vector<std::vector<std::string>> &unitWords)
{
    LearnerState state = defaultState(today);
    SeededRandom rnd("demo");
    const auto randomInt = [&rnd](double scale) { return static_cast<int>(std::lround(rnd.next() * scale)); };

    Date start = today;
    for (int back = 0; back < 16;) {
        start = addDays(start, -1);
        if (isStudyDay(start, state.plan))
            ++back;
    }
    state.plan.start = start;

    // One missed study day, covered by a streak freeze, so the demo shows how freezes look.
    const Date skipped = nthStudyDay(start, 7, state.plan).value();
    state.frozen.insert(skipped);

    static const std::pair<const char *, int> kSkills[]{{"vocab", 8}, {"grammar", 6}, {"listening", 3}, {"verbs", 10}, {"past", 4}};
    for (Date d = start; d < today; d = addDays(d, 1)) {
        const bool study = isStudyDay(d, state.plan);
        if ((!study && rnd.next() > 0.35) || d == skipped)
            continue;
        Activity &activity = state.activity[d];
        activity.seconds = 1200 + randomInt(1500);
        activity.inputMinutes = 20 + randomInt(50);
        activity.islandMinutes = rnd.next() > 0.6 ? 30 : 0;
        activity.speak = 6 + randomInt(6);
        for (const auto &[skill, n] : kSkills) {
            const int total = static_cast<int>(std::lround(n * (0.6 + rnd.next())));
            const int correct = static_cast<int>(std::lround(total * (0.7 + rnd.next() * 0.25)));
            activity.answers[skill] = {correct, total};
        }
        // The listening log, as the web demo writes it (the same draw keeps both on one sequence).
        state.input.push_back({d, activity.inputMinutes, "podcast", rnd.next() > 0.5 ? "Dreaming Spanish" : "No Hay Tos"});
        addXp(state, 60 + randomInt(90), d);
        if (study) {
            const PlanStatus status = planStatus(state);
            if (status.entry.type == DayType::Checkpoint)
                saveCheckpoint(state, status.entry.phase, 0.87, d);
            for (const Task task : status.tasks) {
                if (task == Task::Lesson) {
                    static const std::vector<std::string> noWords;
                    const auto unit = static_cast<std::size_t>(status.entry.unit);
                    const double score = 0.8 + rnd.next() * 0.2;
                    learnUnit(state, status.entry.unit, unit < unitWords.size() ? unitWords[unit] : noWords, score, d);
                } else {
                    markTask(state, task, d);
                }
            }
        }
    }

    // Age the cards so the review queue looks realistic, in the order they joined the deck.
    std::vector<Card *> deck;
    for (auto &[id, card] : state.cards)
        deck.push_back(&card);
    std::sort(deck.begin(), deck.end(), [](const Card *a, const Card *b) { return a->seq < b->seq; });
    for (Card *card : deck) {
        const int age = diffDays(card->added, today);
        card->box = std::min(5, 1 + age / 4);
        card->reps = card->box;
        card->due = addDays(today, static_cast<int>(std::floor(rnd.next() * 5)) - 1);
    }

    // Three devices in Spanish and one conversation, as in the web demo. Its minutes count on that day only if
    // the learner studied then, so the XP of each day stays the web demo's.
    state.missions = {{"phone", start}, {"siri", start}, {"keyboard", addDays(start, 2)}};
    state.exchanges.push_back({addDays(today, -3), 45, "Mariana (Tandem)", "Hablamos de comida mexicana."});
    if (const auto day = state.activity.find(addDays(today, -3)); day != state.activity.end())
        day->second.talkMinutes += 45;
    checkAchievements(state, today);

    // Two reflections, so the coach has a taste to show. Desktop only: the web app has no coach, and no draw is made.
    state.sessions = 2 * kReflectEvery + 1;
    state.reflectedAt = 2 * kReflectEvery;
    state.reflections = {
        {addDays(today, -6), "mixed", {"choice", "fill", "listen", "order"}, {"listen", "order"}, Feel::Right, ""},
        {addDays(today, -2), "lesson", {"match", "choice", "order", "listen", "gender", "speak"}, {"listen", "speak"}, Feel::Right, ""},
    };
    return state;
}

} // namespace lq
