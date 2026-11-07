#pragma once

#include "lq/course.h"
#include "lq/date.h"
#include "lq/progress.h"
#include "lq/random.h"
#include "lq/text.h"
#include "lq/verbs.h"

#include <string>
#include <utility>
#include <vector>

// The exercise engine, ported from app/js/ui/exercises.js: builders turn course content into items, and a
// Session plays any list of them (lessons, trainers, review, checkpoints). Wrong answers come back once.
namespace lq {

enum class ItemType { Choice, Fill, Type, Conjugate, Order, Listen, Match, Gender, Card, Speak, Describe };

// The kind of exercise as the interface and the reflections name it: "choice", "fill", "type", "conj", "order",
// "listen", "match", "gender", "card", "speak" or "describe".
std::string_view itemKind(ItemType type);

// One exercise. Which fields are used depends on the type.
struct Item {
    ItemType type = ItemType::Choice;
    std::string skill;                 // what the answer counts towards: "vocab", "grammar", "verbs"…
    std::string question;              // Choice and Fill (with ___); Type: the English word
    std::string prompt;                // Type: the instruction
    std::vector<std::string> options;  // Choice, in display order
    int answer = 0;                    // Choice: index of the right option
    std::vector<std::string> answers;  // Fill, Type, Conjugate: accepted answers
    std::string hint;
    std::string why;
    std::string note;   // Gender: why the word breaks the rule
    std::string audio;  // Choice: the Spanish to play
    std::string es;     // Order, Listen, Speak: the sentence; Gender, Card: the word; Describe: the word asked for
    std::string en;
    std::vector<std::string> tiles;                          // Order, Listen
    std::vector<std::pair<std::string, std::string>> pairs;  // Match: Spanish, English
    std::string word;                   // Gender: without its article
    std::string article;                // Gender: the right one
    std::vector<std::string> articles;  // Gender: the two choices
    std::string cardId;                 // Card
    std::string example;                // Card
    char gender = 0;                    // Card
    std::string model;                  // Describe: a model answer
    std::string verb;                   // Conjugate
    Tense tense = Tense::Present;
    int person = 0;
    std::string pronoun;
    int unit = -1;       // the unit an exercise or card comes from
    bool retry = false;  // the second try of a missed item
};

// ---- Builders: each mirrors the web function of the same name. -----------------------------------------
Item choiceItem(std::string question, const std::vector<std::string> &options, SeededRandom &rnd);  // first is right
std::vector<std::string> sentenceTiles(std::string_view es, const std::vector<std::string> &pool, SeededRandom &rnd);
std::vector<std::string> vocabPool(const Unit *unit);
Item fromRaw(const RawExercise &raw, const Unit *unit, SeededRandom &rnd);
Item genderItem(const std::string &es, const std::string &en, const std::string &note = {});
Item vocabItem(const Vocab &word, const std::vector<Vocab> &list, SeededRandom &rnd);

std::vector<Item> lessonItems(const Course &course, int unit, SeededRandom &rnd);
std::vector<Item> mixedItems(const Course &course, const std::vector<int> &units, int n, SeededRandom &rnd);
std::vector<Item> conjugationItems(const std::vector<std::string_view> &verbList, const std::vector<Tense> &tenses, int n,
                                   bool vosotros, SeededRandom &rnd);
std::vector<Item> pastItems(const Course &course, int n, SeededRandom &rnd);
std::vector<Item> cognateItems(const Course &course, int n, SeededRandom &rnd);
std::vector<Item> falseFriendItems(const Course &course, int n, SeededRandom &rnd);
std::vector<Item> genderItems(const Course &course, int n, SeededRandom &rnd);
std::vector<Item> cardItems(const Course &course, const std::vector<std::string> &ids);
std::vector<Item> speakItems(const std::vector<std::string> &phrases);
std::vector<Item> describeItems(const Course &course, int n, SeededRandom &rnd);
// Phrases from finished lessons plus today's pronunciation group.
std::vector<Item> speakingItems(const Course &course, const std::vector<int> &completedUnits, Date today, SeededRandom &rnd);
// Phases 1–4: 15 questions from the phase's lessons, verbs and vocabulary.
std::vector<Item> checkpointItems(const Course &course, int phase, SeededRandom &rnd);
// The B1 mock: reading and listening questions, and 10 grammar items from phases 3–5.
std::vector<Item> mockQuestions(const MockPart &part, const std::string &skill, SeededRandom &rnd);
std::vector<Item> mockGrammarItems(const Course &course, SeededRandom &rnd);

std::vector<int> completedUnits(const LearnerState &state);
// The present, plus every tense whose lesson is done.
std::vector<Tense> unlockedTenses(const LearnerState &state);
// Today's group in the pronunciation gym (the same all day, as on the web).
int pronunciationGroupFor(const Course &course, Date today);

// ---- Checking ---------------------------------------------------------------------------------------
struct Response {
    int choice = -1;          // Choice
    std::string text;         // Fill, Type, Conjugate
    std::vector<int> picked;  // Order, Listen: tile indices in order
};

bool canCheck(const Item &item, const Response &response);
AnswerCheck checkItem(const Item &item, const Response &response);
// The right answer shown after a miss ("" for Match).
std::string correctAnswer(const Item &item);
// The full Spanish sentence to listen to once an item is answered ("" when there is none).
std::string sentenceOf(const Item &item);

// ---- Session ---------------------------------------------------------------------------------------
class Session
{
public:
    explicit Session(std::vector<Item> items = {}, bool requeue = true);

    bool finished() const { return m_position >= m_queue.size(); }
    const Item &current() const { return m_queue[m_position]; }
    std::size_t position() const { return m_position; }
    std::size_t size() const { return m_queue.size(); }
    // The second-try round: once every item has had its turn, the first misses come back, in the order missed.
    bool secondTry() const { return m_position >= m_firstRound && m_queue.size() > m_firstRound; }
    // Where the current round is, and how long it is.
    std::size_t roundPosition() const { return secondTry() ? m_position - m_firstRound : m_position; }
    std::size_t roundSize() const { return secondTry() ? m_queue.size() - m_firstRound : m_firstRound; }

    // Counts a checked answer. A first miss is kept for the second-try round; returns true when it is.
    bool answer(bool ok, SeededRandom &rnd);
    // Speaking, describing and review cards are rated by the learner, not checked.
    void selfRated() { ++m_selfRated; }
    // A review card graded "again" is seen once more, in the second-try round.
    void again();
    // Moves on. After the last item of the first round, the kept misses make up the second-try round.
    void next();
    // Ends the session without the rest of the second-try round.
    void skipSecondTry() { m_position = m_queue.size(); }

    int answered() const { return m_answered; }
    int firstTry() const { return m_first; }
    int selfRatedCount() const { return m_selfRated; }
    int secondTries() const { return m_secondTries; }  // checked answers in the second-try round
    int fixed() const { return m_fixed; }              // those that were right
    const std::vector<Item> &missed() const { return m_missed; }
    double score() const { return m_answered ? static_cast<double>(m_first) / m_answered : 1.0; }

private:
    std::vector<Item> m_queue;
    std::size_t m_firstRound = 0;
    std::vector<Item> m_kept;  // waiting for the second-try round
    std::size_t m_position = 0;
    bool m_requeue = true;
    int m_answered = 0;
    int m_first = 0;
    int m_selfRated = 0;
    int m_secondTries = 0;
    int m_fixed = 0;
    std::vector<Item> m_missed;
};

// ---- B1 mock exam --------------------------------------------------------------------------------------
struct WritingCheck {
    bool ok = false;
    std::string label;
};

// Length, past tenses, connectors and a future plan, checked automatically in the writing task.
std::vector<WritingCheck> writingChecks(std::string_view text);
inline constexpr int kWritingSelfChecks = 3;
inline constexpr int kSpeakingRubricMax = 3;  // 0 "could not do it" … 3 "fairly fluent"

struct MockScores {
    Tally reading;
    Tally listening;
    Tally grammar;
    int writingPoints = 0;      // automatic checks passed + self checks ticked, out of 7
    std::vector<int> speaking;  // one rubric rating per task
};

struct MockResult {
    double reading = 0;  // reading and grammar together
    double listening = 0;
    double writing = 0;
    double speaking = 0;
    double written = 0;  // DELE groups: reading + writing, listening + speaking
    double oral = 0;
    double overall = 0;
    bool pass = false;  // 60% or more in each group
};

MockResult mockResult(const MockScores &scores);

} // namespace lq
