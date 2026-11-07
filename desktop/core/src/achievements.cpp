#include "lq/achievements.h"

#include <cmath>
#include <cstdio>
#include <functional>

namespace lq {

namespace {

struct Rule {
    Achievement achievement;
    std::function<bool(const LearnerState &, const Totals &, int longest)> earned;
};

int correct(const Totals &totals, std::string_view skill)
{
    const auto it = totals.answers.find(skill);
    return it == totals.answers.end() ? 0 : it->second.correct;
}

double best(const LearnerState &state, int phase)
{
    const auto it = state.checkpoints.find(phase);
    return it == state.checkpoints.end() ? -1.0 : it->second.best;
}

// The same 16 as the web app, in the same order.
const std::vector<Rule> &rules()
{
    static const std::vector<Rule> list{
        {{"first_lesson", "book", "First Lesson", "Finish a lesson."},
         [](const LearnerState &s, const Totals &, int) { return !s.units.empty(); }},
        {{"streak_5", "flame", "5-Day Streak", "Keep a streak for 5 days."},
         [](const LearnerState &, const Totals &, int longest) { return longest >= 5; }},
        {{"streak_15", "flame", "15-Day Streak", "Keep a streak for 15 days."},
         [](const LearnerState &, const Totals &, int longest) { return longest >= 15; }},
        {{"streak_30", "flame", "30-Day Streak", "Keep a streak for 30 days."},
         [](const LearnerState &, const Totals &, int longest) { return longest >= 30; }},
        {{"verbs_100", "bolt", "100 Verb Forms", "Conjugate 100 verb forms right."},
         [](const LearnerState &, const Totals &t, int) { return correct(t, "verbs") >= 100; }},
        {{"past_master", "clock", "Fui vs. Iba", "85 % right over 30 or more preterite-or-imperfect answers."},
         [](const LearnerState &, const Totals &t, int) {
             return t.answered("past") >= 30 && t.accuracy("past").value_or(0) >= 0.85;
         }},
        // False friends were counted as cognates until they got a skill of their own; they still count here.
        {{"cognates_50", "sparkles", "Cognate Hunter", "Get 50 cognates right."},
         [](const LearnerState &, const Totals &t, int) { return correct(t, "cognates") + correct(t, "falsefriends") >= 50; }},
        {{"words_100", "cards", "100 Words Remembered", "Remember 100 words in your review deck."},
         [](const LearnerState &, const Totals &t, int) { return t.wordsRemembered >= 100; }},
        {{"input_10h", "headphones", "10 h Listening", "Log 10 hours of Spanish listening."},
         [](const LearnerState &, const Totals &t, int) { return t.inputMinutes >= 600; }},
        {{"input_50h", "headphones", "50 h Listening", "Log 50 hours of Spanish listening."},
         [](const LearnerState &, const Totals &t, int) { return t.inputMinutes >= 3000; }},
        {{"island_5h", "palm", "Island Dweller", "Spend 5 hours on the language island, with no English."},
         [](const LearnerState &, const Totals &t, int) { return t.islandMinutes >= 300; }},
        {{"devices", "globe", "Goodbye English", "Switch 5 of your devices or apps to Spanish."},
         [](const LearnerState &s, const Totals &, int) { return s.missions.size() >= 5; }},
        {{"exchange_1", "users", "First Exchange", "Log your first conversation with a Spanish speaker."},
         [](const LearnerState &s, const Totals &, int) { return !s.exchanges.empty(); }},
        {{"phase_1", "route", "Foundations Done", "Pass the phase 1 checkpoint."},
         [](const LearnerState &s, const Totals &, int) { return s.checkpoints.contains(1); }},
        {{"phase_3", "route", "Past Tense Unlocked", "Pass the phase 3 checkpoint."},
         [](const LearnerState &s, const Totals &, int) { return s.checkpoints.contains(3); }},
        {{"b1_mock", "award", "B1 Mock Passed", "Score 70 % or more on the B1 mock exam."},
         [](const LearnerState &s, const Totals &, int) { return best(s, 5) >= 0.7; }},
    };
    return list;
}

std::string percent(std::optional<double> value)
{
    return value ? std::to_string(static_cast<int>(std::lround(*value * 100))) + "%" : "–";
}

std::string hours(int minutes)
{
    char text[32];
    std::snprintf(text, sizeof text, "%.1f h", minutes / 60.0);
    return text;
}

} // namespace

const std::vector<Achievement> &achievementList()
{
    static const std::vector<Achievement> list = [] {
        std::vector<Achievement> out;
        for (const Rule &rule : rules())
            out.push_back(rule.achievement);
        return out;
    }();
    return list;
}

std::vector<Achievement> checkAchievements(LearnerState &state, Date today)
{
    const Totals t = totals(state, today);
    const int longest = streakInfo(state, today).longest;
    std::vector<Achievement> fresh;
    for (const Rule &rule : rules()) {
        if (!state.achievements.contains(rule.achievement.id) && rule.earned(state, t, longest)) {
            state.achievements.emplace(std::string(rule.achievement.id), today);
            fresh.push_back(rule.achievement);
        }
    }
    return fresh;
}

std::vector<ReadinessSignal> readiness(const LearnerState &state, const Totals &t)
{
    const auto lessons = static_cast<int>(state.units.size());
    return {
        {lessons >= 25, "All 25 lessons (" + std::to_string(lessons) + "/25)"},
        {t.answered("past") >= 30 && t.accuracy("past").value_or(0) >= 0.85,
         "Fui vs. iba: 85%+ over 30+ answers (" + percent(t.accuracy("past")) + ")"},
        {t.answered("verbs") >= 100 && t.accuracy("verbs").value_or(0) >= 0.8,
         "Verb forms: 80%+ over 100+ answers (" + percent(t.accuracy("verbs")) + ")"},
        {t.wordsRemembered >= 200, "200+ words remembered (" + std::to_string(t.wordsRemembered) + ")"},
        {t.inputMinutes >= 3000, "50+ hours of listening (" + hours(t.inputMinutes) + ")"},
        {state.exchanges.size() >= 5, "5+ real conversations (" + std::to_string(state.exchanges.size()) + ")"},
        {best(state, 5) >= 0.7, "B1 mock exam at 70%+"},
    };
}

} // namespace lq
