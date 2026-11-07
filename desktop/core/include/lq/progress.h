#pragma once

#include "lq/calendar.h"
#include "lq/srs.h"
#include "lq/streak.h"

#include <array>
#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

namespace lq {

// XP rewards, as in the web app (app/js/core/progress.js).
inline constexpr int kXpCorrect = 10;
inline constexpr int kXpAlmost = 6;   // right but for the accents
inline constexpr int kXpRetry = 8;    // right in the second-try round (web: 5); below kXpCorrect, so a miss never pays
inline constexpr int kXpCard = 2;     // a review card remembered (1 for "again")
inline constexpr int kXpPhrase = 5;   // a phrase said out loud
inline constexpr int kXpLessonBonus = 20;
inline constexpr int kXpDayBonus = 20;

struct Tally {
    int correct = 0;
    int total = 0;
    bool operator==(const Tally &) const = default;
};

struct Activity {
    int xp = 0;
    int seconds = 0;        // active time in the app
    int inputMinutes = 0;   // Spanish listening outside the app
    int islandMinutes = 0;  // time spent thinking only in Spanish
    int talkMinutes = 0;    // conversations with people (language exchanges)
    int speak = 0;          // phrases said out loud
    std::map<std::string, Tally, std::less<>> answers;  // by skill: "vocab", "grammar", "verbs"…
    bool operator==(const Activity &) const = default;
};

struct DoneDay {
    int n = 0;    // plan day
    Date date{};  // calendar day it was completed
    bool operator==(const DoneDay &) const = default;
};

struct Score {
    Date date{};  // first completion
    double score = 0;
    double best = 0;
    int attempts = 0;
    bool operator==(const Score &) const = default;
};

enum class Feel { Easy, Right, Hard };

// What the learner said after a session: the kinds of exercise they enjoyed and how hard it felt.
struct Reflection {
    Date date{};
    std::string activity;              // "lesson", a trainer ("verbs", "mixed"…) or "coach" outside a session
    std::vector<std::string> offered;  // exercise kinds there were to choose from: "listen", "order"…
    std::vector<std::string> liked;    // the ones the learner enjoyed
    std::optional<Feel> feel;          // not asked outside a session
    std::string note;                  // optional, in their own words
    bool operator==(const Reflection &) const = default;
};

// Spanish heard outside the app, logged on the Immersion screen.
struct InputEntry {
    Date date{};
    int minutes = 0;
    std::string kind;    // one of inputKinds(): "podcast", "video", "series", "reading", "music", "other"
    std::string source;  // what it was, in the learner's words; may be empty
    // Read from a streaming service's viewing history (lq/viewing.h): it counts as listening on the day it was
    // watched, but earns no XP, so importing old viewing never changes a streak after the fact.
    bool imported = false;
    bool operator==(const InputEntry &) const = default;
};

// A conversation with a Spanish speaker (Tandem, HelloTalk, a coworker…).
struct Exchange {
    Date date{};
    int minutes = 0;
    std::string partner;
    std::string note;
    bool operator==(const Exchange &) const = default;
};

// Everything the engine needs to know about one learner. Metrics (streak, pace) are derived from it.
struct LearnerState {
    Plan plan;
    std::vector<DoneDay> done;                  // completed plan days, in order
    std::map<int, std::map<Task, Date>> tasks;  // plan day → task → date completed
    std::map<int, Score> units;                 // unit index (0..24) → lesson result
    std::map<int, Score> checkpoints;           // phase (1..5, 5 = B1 mock) → result
    std::map<Date, Activity> activity;          // per calendar day
    Deck cards;                                 // spaced-repetition deck: words join it with their lesson
    int freezes = 0;                            // streak freezes held
    std::set<Date> frozen;                      // missed study days covered by a freeze
    int longest = 0;                            // longest streak seen when it was extended
    int sessions = 0;                           // lessons and practice rounds finished
    int reflectedAt = 0;                        // `sessions` when the learner last reflected or said "not now"
    std::vector<Reflection> reflections;        // oldest first
    std::vector<InputEntry> input;              // listening log, oldest first
    std::vector<Exchange> exchanges;            // oldest first
    std::map<std::string, Date, std::less<>> missions;      // device switched to Spanish → when
    std::map<std::string, Date, std::less<>> achievements;  // achievement id → when it was earned
    bool operator==(const LearnerState &) const = default;
};

enum class EventType { FreezeEarned, Streak, FreezeUsed, DayComplete, PhaseComplete };

// What a state change means for the interface (celebrations, toasts).
struct Event {
    EventType type;
    int value = 0;  // freezes gained, streak length, plan day or phase
    std::vector<Date> dates = {};
};
using Events = std::vector<Event>;

LearnerState defaultState(Date today);
const std::vector<ScheduleEntry> &schedule(int totalDays);  // totalDays: 30, 60 or 90
std::set<Date> activeDays(const LearnerState &state);
int streakCount(const LearnerState &state, Date today);

Events addXp(LearnerState &state, int xp, Date today);
// Applied when the app opens: covers missed study days since the last active day with freezes.
Events applyFreezes(LearnerState &state, Date today);

// One checked answer: counts it for the skill and rewards it (less for accents or in the second-try round).
Events recordAnswer(LearnerState &state, std::string_view skill, bool ok, bool almost, bool retry, Date today);
// A phrase said out loud in a speaking exercise.
Events recordPhrase(LearnerState &state, Date today);
// A review card graded by the learner. Cards not in the deck yet join it.
Events gradeReview(LearnerState &state, const std::string &id, int unit, Grade grade, Date today);

enum class DayState { Done, Frozen, Future, Today, Missed, Off };

struct WeekDay {
    Date date{};
    DayState state = DayState::Off;
    bool scheduled = false;
};

struct StreakInfo {
    int count = 0;
    int freezes = 0;
    bool todayDone = false;
    std::array<WeekDay, 7> week{};  // Monday to Sunday of the current week
    int longest = 0;
};

StreakInfo streakInfo(const LearnerState &state, Date today);

struct PlanStatus {
    int n = 0;  // current plan day
    ScheduleEntry entry;
    std::vector<Task> tasks;
    int total = 0;
    int doneCount = 0;
    bool finished = false;
    int minutes = 0;
};

PlanStatus planStatus(const LearnerState &state);
bool taskDone(const LearnerState &state, int n, Task task);
Events markTask(LearnerState &state, Task task, Date today);
// Completes the current plan day once every task is done.
Events refreshDay(LearnerState &state, Date today);
// Saves the lesson result and adds the unit's words to the review deck.
Events learnUnit(LearnerState &state, int unit, const std::vector<std::string> &words, double score, Date today);
Events saveCheckpoint(LearnerState &state, int phase, double score, Date today);

// Review is a no-op when nothing is due, so it ticks itself; a tick from an earlier "get ahead" day is
// removed when new cards have come due since. Run after every change, as the web app does on each render.
struct ReviewSync {
    bool changed = false;
    Events events;
};
ReviewSync syncReview(LearnerState &state, Date today);

struct Pace {
    int doneCount = 0;
    int expected = 0;  // study days elapsed since the start, up to yesterday
    int diff = 0;      // negative: behind
    int remaining = 0;
    std::optional<Date> target;  // date the plan ends at one plan day per study day
};

Pace pace(const LearnerState &state, Date today);

// Plan length suggested for a self-assessed level ("A1", "A2", "A2+"), as on the web app's welcome screen.
int recommendedPlanDays(std::string_view level);

// Checkpoints that show a level for the series recommended: the phase 2 checkpoint closes the everyday present
// (A2), the phase 4 one the past, future and conditional (B1). Passing is the checkpoint's own 70%.
inline constexpr int kSeriesA2Phase = 2;
inline constexpr int kSeriesB1Phase = 4;
inline constexpr double kCheckpointPass = 0.7;

// The level of the series to recommend, "A1", "A2" or "B1": the level chosen at the welcome screen (A2+ counts
// as A2), raised by the checkpoints passed. It never goes down. The web app's progress.js seriesLevel is the same.
std::string seriesLevel(std::string_view startLevel, const LearnerState &state);

// Every few lessons or practice rounds the app asks what the learner enjoyed, so practice can lean towards it.
inline constexpr int kReflectEvery = 3;
inline constexpr std::size_t kMaxNoteLength = 280;  // characters

// A lesson or practice round with at least one exercise. Checkpoints and the mock exam are tests: they do not count.
void countSession(LearnerState &state);
bool reflectionDue(const LearnerState &state);
void addReflection(LearnerState &state, Reflection reflection);
// "Not now": the question comes back after kReflectEvery more sessions.
void postponeReflection(LearnerState &state);
std::string_view feelId(Feel feel);  // "easy", "right", "hard"
std::optional<Feel> feelFromId(std::string_view id);

// ---- Immersion --------------------------------------------------------------------------------------------
// Minutes logged count as Spanish time and earn their number in XP, as in the web app. The language island
// (an hour with no English) is logged the same way, as islandMinutes.
inline constexpr int kMaxLoggedMinutes = 600;
inline constexpr int kMaxExchangeMinutes = 300;
inline constexpr std::size_t kMaxLogText = 140;  // characters in a source, partner or note

// The kinds of listening the log offers, plus "island" for the language island.
const std::vector<std::string_view> &inputKinds();
// minutes is clamped to 1..kMaxLoggedMinutes; an unknown kind is "other"; texts are trimmed and cut.
Events logInput(LearnerState &state, int minutes, std::string_view kind, std::string_view source, Date today);
// Takes an entry back, with its minutes and XP from the day it was logged (an imported one gave no XP).
// False for an index out of range.
bool removeInput(LearnerState &state, std::size_t index);
Events logExchange(LearnerState &state, int minutes, std::string_view partner, std::string_view note, Date today);
void setMission(LearnerState &state, std::string_view id, bool done, Date today);

// Dreaming Spanish's roadmap measures progress in hours of listening; its levels start at these.
inline constexpr std::array<int, 7> kListeningMilestones{0, 50, 150, 300, 600, 1000, 1500};

struct ListeningLevel {
    int level = 1;         // 1..7
    int nextHours = 0;     // the next milestone; 0 past the last one
    double progress = 0;   // from the current milestone to the next, 0..1
};

ListeningLevel listeningLevel(int inputMinutes);

// ---- Totals ---------------------------------------------------------------------------------------------
struct Totals {
    int xp = 0;
    double appMinutes = 0;
    int inputMinutes = 0;
    int islandMinutes = 0;
    int talkMinutes = 0;
    int speak = 0;
    std::map<std::string, Tally, std::less<>> answers;  // all time, by skill
    int wordsSeen = 0;        // in the deck
    int wordsRemembered = 0;  // box kRememberedBox or above
    int wordsDue = 0;

    double spanishMinutes() const { return appMinutes + inputMinutes + islandMinutes + talkMinutes; }
    int answered(std::string_view skill) const;
    // Right on the first try, out of every answer; empty before the first one.
    std::optional<double> accuracy(std::string_view skill) const;
};

Totals totals(const LearnerState &state, Date today);
// Minutes in Spanish on one day: app time, listening, island and conversations.
int spanishMinutes(const LearnerState &state, Date day);

} // namespace lq
