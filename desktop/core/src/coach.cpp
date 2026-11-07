#include "lq/coach.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <set>
#include <utility>

namespace lq {

namespace {

int percent(double ratio)
{
    return static_cast<int>(std::lround(ratio * 100));
}

std::string days(int n)
{
    return n == 1 ? std::string("1 day") : std::to_string(n) + " days";
}

// How the coach names a trainer inside a sentence. Trainer ids and plan task ids are the same.
std::string trainerName(std::string_view trainer)
{
    static const std::map<std::string_view, std::string_view> names{
        {"review", "Review"},
        {"verbs", "the Verb gym"},
        {"mixed", "Mixed practice"},
        {"speak", "Shadowing"},
        {"cognates", "the COGNATES LAB"},
        {"gender", "the El / La trainer"},
        {"past", "Fui vs. Iba"},
        {"falsefriends", "False friends"},
        {"describe", "Describe it in Spanish"},
    };
    const auto it = names.find(trainer);
    return std::string(it == names.end() ? trainer : it->second);
}

std::string taskName(Task task, const ScheduleEntry &entry, const Course &course)
{
    switch (task) {
    case Task::Lesson: {
        const Unit &unit = course.units.at(static_cast<std::size_t>(entry.unit));
        return "Lesson " + std::to_string(unit.n) + ", " + unit.title;
    }
    case Task::Checkpoint: return "the phase " + std::to_string(entry.phase) + " checkpoint";
    case Task::Final: return "the B1 mock exam";
    default: return trainerName(taskId(task));
    }
}

// Typed, built and spoken answers ask more of the learner than recognising the right one among several.
bool producing(std::string_view kind)
{
    for (const std::string_view k : {"type", "fill", "conj", "order", "listen", "speak", "describe"}) {
        if (k == kind)
            return true;
    }
    return false;
}

double feelFactor(std::string_view kind, std::optional<Feel> feel)
{
    if (!feel || *feel == Feel::Right)
        return 1.0;
    return producing(kind) == (*feel == Feel::Easy) ? 1.3 : 0.7;
}

// The skills the coach judges, what it calls them, and the trainer that works on each.
struct SkillCoach {
    std::string_view skill, name, trainer, why;
};

constexpr SkillCoach kSkillCoaches[]{
    {"verbs", "verb endings", "verbs", "The Verb gym drills the endings of the tenses you have unlocked."},
    {"past", "the past tenses", "past", "Fui vs. Iba practises exactly that choice."},
    {"gender", "el and la", "gender", "The El / La trainer has the rules and the famous exceptions."},
    {"cognates", "cognates", "cognates", "The COGNATES LAB turns English endings into Spanish ones."},
    {"falsefriends", "false friends", "falsefriends", "False friends drills the words that only look like English ones."},
    {"grammar", "grammar", "mixed", "Mixed practice brings back the grammar of every lesson you have done."},
    {"listening", "listening", "speak", "Shadowing trains your ear: listen, then repeat out loud."},
    {"vocab", "vocabulary", "review", "Review brings words back just before you forget them."},
};

// The trainer built on each kind of exercise, for an extra round the learner will enjoy.
constexpr std::pair<std::string_view, std::string_view> kKindTrainers[]{
    {"conj", "verbs"},   {"type", "cognates"},  {"gender", "gender"},      {"speak", "speak"},
    {"describe", "describe"}, {"card", "review"}, {"choice", "falsefriends"}, {"fill", "mixed"},
    {"order", "mixed"},  {"listen", "mixed"},   {"match", "mixed"},
};

} // namespace

double Taste::weight(std::string_view kind) const
{
    const auto it = weights.find(kind);
    return it == weights.end() ? 1.0 : it->second;
}

std::vector<std::string> Taste::favourites() const
{
    std::vector<std::pair<std::string, double>> liked;
    for (const auto &[kind, w] : weights) {
        if (w > kFavourite)
            liked.emplace_back(kind, w);
    }
    std::stable_sort(liked.begin(), liked.end(), [](const auto &a, const auto &b) { return a.second > b.second; });
    std::vector<std::string> kinds;
    for (auto &entry : liked)
        kinds.push_back(std::move(entry.first));
    return kinds;
}

Taste taste(const LearnerState &state)
{
    Taste result;
    std::map<std::string, std::pair<int, int>, std::less<>> counts;  // kind → times enjoyed, times offered
    const std::size_t total = state.reflections.size();
    for (std::size_t i = total > kTasteMemory ? total - kTasteMemory : 0; i < total; ++i) {
        const Reflection &r = state.reflections[i];
        std::set<std::string, std::less<>> offered(r.offered.begin(), r.offered.end());
        offered.insert(r.liked.begin(), r.liked.end());
        const std::set<std::string, std::less<>> liked(r.liked.begin(), r.liked.end());
        for (const std::string &kind : offered) {
            auto &[enjoyed, times] = counts[kind];
            ++times;
            if (liked.contains(kind))
                ++enjoyed;
        }
        if (r.feel) {
            result.feel = r.feel;
            result.feltOn = r.date;
        }
        ++result.reflections;
    }
    // Laplace's rule of succession: one reflection moves a weight a little, a steady pattern moves it more.
    for (const auto &[kind, count] : counts)
        result.weights[kind] = 0.5 + (count.first + 1.0) / (count.second + 2.0);
    return result;
}

std::string_view kindLabel(std::string_view kind)
{
    static const std::map<std::string_view, std::string_view> labels{
        {"choice", "multiple choice"},  {"fill", "filling the gap"},     {"type", "typing the word"},
        {"conj", "conjugating verbs"},  {"order", "building sentences"}, {"listen", "listening"},
        {"match", "matching pairs"},    {"gender", "el or la"},          {"card", "flashcards"},
        {"speak", "speaking out loud"}, {"describe", "describing in Spanish"},
    };
    const auto it = labels.find(kind);
    return it == labels.end() ? kind : it->second;
}

std::string tasteSummary(const Taste &taste)
{
    const std::vector<std::string> favourites = taste.favourites();
    std::string text;
    if (favourites.empty()) {
        text = "Noted. Pick what you enjoy next time, and mixed practice will lean towards it.";
    } else {
        text = "Mixed practice now leans towards " + std::string(kindLabel(favourites[0]));
        if (favourites.size() > 1)
            text += " and " + std::string(kindLabel(favourites[1]));
        text += '.';
    }
    if (taste.feel == Feel::Easy)
        text += " After an easy session it also asks you to type and build more answers.";
    else if (taste.feel == Feel::Hard)
        text += " After a hard session it also gives you more answers to choose from, until it feels right.";
    return text;
}

std::vector<Item> preferredItems(std::vector<Item> pool, std::size_t n, const Taste &taste, SeededRandom &rnd)
{
    std::vector<Item> picked;
    while (picked.size() < n && !pool.empty()) {
        std::vector<double> weights;
        double sum = 0;
        for (const Item &item : pool) {
            const std::string_view kind = itemKind(item.type);
            weights.push_back(taste.weight(kind) * feelFactor(kind, taste.feel));
            sum += weights.back();
        }
        double target = rnd.next() * sum;
        std::size_t chosen = pool.size() - 1;
        for (std::size_t i = 0; i < weights.size(); ++i) {
            if (target < weights[i]) {
                chosen = i;
                break;
            }
            target -= weights[i];
        }
        picked.push_back(std::move(pool[chosen]));
        pool.erase(pool.begin() + static_cast<std::ptrdiff_t>(chosen));
    }
    return picked;
}

std::vector<CoachTip> coachTips(const LearnerState &state, const Course &course, Date today, std::string_view after)
{
    std::vector<CoachTip> tips;
    const auto add = [&tips, after](std::string id, std::string title, std::string detail, std::string trainer = {},
                                    int unit = -1, std::optional<Task> task = std::nullopt) {
        if (trainer.empty() || trainer != after)
            tips.push_back({std::move(id), std::move(title), std::move(detail), std::move(trainer), unit, task});
    };
    const PlanStatus status = planStatus(state);
    const Pace progress = pace(state, today);
    const int due = static_cast<int>(reviewQueue(state.cards, today, std::numeric_limits<int>::max()).size());
    const bool started = !state.units.empty();
    const bool doneToday = std::any_of(state.done.begin(), state.done.end(), [today](const DoneDay &d) { return d.date == today; });
    const Taste learned = taste(state);

    // Due words are cheap to keep and expensive to relearn.
    if (due >= kReviewPile) {
        add("review",
            "Clear your review pile first",
            std::to_string(due) + " cards are due. Words come back just before you forget them, so ten minutes "
                "today saves relearning them next week.",
            "review");
    }

    if (!status.finished && progress.diff <= -2) {
        add("behind",
            "Catch up one plan day at a time",
            "You are " + days(-progress.diff) + " behind. Finish today's plan, then carry on with the next plan day "
                "when you have time to spare. There is no need to catch up all at once.");
    }

    // A checkpoint in the next few plan days: mixed practice covers every lesson it asks about.
    if (started && !status.finished) {
        const std::vector<ScheduleEntry> &entries = schedule(state.plan.days);
        for (int n = status.n; n <= std::min(status.n + kCheckpointAhead, status.total); ++n) {
            const ScheduleEntry &entry = entries[static_cast<std::size_t>(n - 1)];
            if (entry.type != DayType::Checkpoint)
                continue;
            // Once today's plan is done, status.n is already the next plan day.
            const int ahead = n - status.n + (doneToday ? 1 : 0);
            const bool exam = entry.phase == kPhases;
            const std::string what = exam ? "The B1 mock exam" : "The phase " + std::to_string(entry.phase) + " checkpoint";
            const std::string when = ahead == 0   ? " is today"
                                     : ahead == 1 ? " is your next plan day"
                                                  : " is in " + std::to_string(ahead) + " plan days";
            add("checkpoint", what + when,
                exam ? "Reading, listening, grammar, writing and speaking. A round of mixed practice warms you up for all of them."
                     : "15 questions from this phase, with no second tries. Mixed practice draws on every lesson you have done: "
                       "the best way to prepare.",
                "mixed");
            break;
        }
    }

    // The weakest skill of the last two weeks, once there are enough answers to judge it.
    std::map<std::string, Tally, std::less<>> recent;
    for (const auto &[date, activity] : state.activity) {
        if (date > today || diffDays(date, today) >= kSkillWindow)
            continue;
        for (const auto &[skill, tally] : activity.answers) {
            recent[skill].correct += tally.correct;
            recent[skill].total += tally.total;
        }
    }
    const SkillCoach *weakest = nullptr;
    double lowest = kWeakSkill;
    for (const SkillCoach &coach : kSkillCoaches) {
        const auto it = recent.find(coach.skill);
        if (it == recent.end() || it->second.total < kSkillMinAnswers)
            continue;
        const double ratio = static_cast<double>(it->second.correct) / it->second.total;
        if (ratio < lowest) {
            lowest = ratio;
            weakest = &coach;
        }
    }
    if (weakest) {
        const Tally &tally = recent.find(weakest->skill)->second;
        std::string trainer(weakest->trainer);
        std::string why(weakest->why);
        if (trainer == "review" && due == 0) {
            trainer = "mixed";
            why = "Mixed practice brings back words from every lesson you have done.";
        }
        add("skill",
            "Give " + std::string(weakest->name) + " some attention",
            std::to_string(percent(lowest)) + "% right over the last two weeks (" + std::to_string(tally.correct)
                + " of " + std::to_string(tally.total) + "). " + why,
            trainer);
    }

    // The lesson with the lowest best score, if it is low; not right after a lesson, when the next one is better.
    if (after != "lesson") {
        const std::pair<const int, Score> *weakLesson = nullptr;
        for (const auto &entry : state.units) {
            if (entry.second.best < kWeakLesson && (!weakLesson || entry.second.best < weakLesson->second.best))
                weakLesson = &entry;
        }
        if (weakLesson) {
            const Unit &unit = course.units.at(static_cast<std::size_t>(weakLesson->first));
            add("lesson",
                "Practise Lesson " + std::to_string(unit.n) + " again",
                "Your best on " + unit.title + " is " + std::to_string(percent(weakLesson->second.best))
                    + "%. A second pass while the words are fresh lifts it quickly.",
                {}, weakLesson->first);
        }
    }

    if (started) {
        Date lastSpoken = state.plan.start;
        for (const auto &[date, activity] : state.activity) {
            if (activity.speak > 0 && date <= today)
                lastSpoken = std::max(lastSpoken, date);
        }
        const int quiet = diffDays(lastSpoken, today);
        if (quiet >= kQuietDays) {
            add("speak",
                "Say something out loud today",
                "Nothing said out loud for " + days(quiet) + ". Shadowing takes 5 minutes and trains your ear as well "
                    "as your mouth.",
                "speak");
        }
    }

    // What the latest reflection said about difficulty, while it is recent.
    if (learned.feel && *learned.feel != Feel::Right && diffDays(learned.feltOn, today) <= kFeelDays) {
        if (*learned.feel == Feel::Hard) {
            add("hard",
                "Your last session felt hard: consolidate it",
                "That is where learning happens. Mixed practice now gives you more answers to choose from, until it "
                    "feels right again.",
                "mixed");
        } else {
            add("easy",
                "Your last session felt easy: stretch yourself",
                "Mixed practice now asks you to type and build more answers. Describe it in Spanish is a good "
                    "stretch too.",
                "mixed");
        }
    }

    // With today's plan done, an extra round of what the learner enjoys.
    if (doneToday || status.finished) {
        for (const std::string &kind : learned.favourites()) {
            const auto it = std::find_if(std::begin(kKindTrainers), std::end(kKindTrainers),
                                         [&kind](const auto &pair) { return pair.first == kind; });
            if (it == std::end(kKindTrainers))
                continue;
            const std::string trainer(it->second);
            if ((trainer == "review" && due == 0) || trainer == after)
                continue;
            add("enjoy",
                "An extra round you'll enjoy: " + trainerName(trainer),
                "You enjoy " + std::string(kindLabel(kind)) + ", and " + trainerName(trainer)
                    + (trainer == "mixed" ? " now leans towards it." : " is built on it.")
                    + " Today's plan is done, so this one is a bonus.",
                trainer);
            break;
        }
    }

    if (!status.finished && progress.diff >= 2) {
        add("ahead",
            "You are " + days(progress.diff) + " ahead",
            "Spend the slack on listening: " + std::to_string(inputTargetMinutes(status.total))
                + " minutes of Spanish audio today, from a podcast, a series or music.");
    }

    // The plan itself, last: its card is right beside the coach, but after a session it is the natural next step.
    if (!status.finished && !doneToday) {
        std::optional<Task> next;
        int left = 0;
        int minutes = 0;
        for (const Task task : status.tasks) {
            if (taskDone(state, status.n, task))
                continue;
            if (!next)
                next = task;
            ++left;
            minutes += taskMinutes(task);
        }
        // Not when a suggestion above already leads to the same task: one reason to open it is enough.
        const auto leadsThere = [&next](const CoachTip &tip) { return tip.trainer == taskId(*next); };
        if (next && taskId(*next) != after && std::ranges::none_of(tips, leadsThere)) {
            add("plan",
                "Next on today's plan: " + taskName(*next, status.entry, course),
                left == 1 ? "It is the last task today, about " + std::to_string(minutes) + " minutes."
                    : std::to_string(left) + " tasks left today, about " + std::to_string(minutes) + " minutes in all.",
                {}, -1, next);
        }
    }
    return tips;
}

} // namespace lq
