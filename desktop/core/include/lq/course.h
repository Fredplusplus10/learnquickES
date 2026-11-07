#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// The course exported from the web app (desktop/content/course.json): 25 units in 5 phases, the trainer
// content and the B1 mock exam. Spanish text is UTF-8.
namespace lq {

struct Vocab {
    std::string id;  // stable across versions: it keys the learner's review cards
    std::string es;
    std::string en;
    std::string example;
    char gender = 0;  // 'm' or 'f' from the article; 0 for words without one
};

// An exercise as written in the course. In multiple choice the first option is the correct one.
struct RawExercise {
    std::string type;  // "mc", "fill", "order" or "listen"
    std::string question;
    std::vector<std::string> options;
    std::vector<std::string> answers;
    std::string hint;
    std::string why;
    std::string es;
    std::string en;
};

struct GrammarTable {
    std::vector<std::string> head;
    std::vector<std::vector<std::string>> rows;
};

struct Grammar {
    std::string title;
    std::string hack;
    std::string tip;
    std::vector<GrammarTable> tables;
    std::vector<std::pair<std::string, std::string>> examples;  // Spanish, English
};

struct DialogueLine {
    std::string speaker;  // "Tú" is the learner
    std::string es;
    std::string en;
};

struct Unit {
    std::string id;  // "u01".."u25"
    int n = 0;
    int phase = 0;
    std::string title;
    std::string titleEs;
    std::string canDo;
    int trick = 0;
    std::string art;
    std::string coach;
    std::vector<Vocab> vocab;
    Grammar grammar;
    std::vector<DialogueLine> dialogue;
    std::vector<RawExercise> exercises;
    std::vector<std::string> speak;
};

struct Phase {
    int n = 0;
    std::string title;
    std::string titleEs;
    std::string description;
};

struct PastItem {
    std::string question;  // with ___
    std::string right;
    std::string wrong;
    std::string reason;  // key into Course::pastReasons
};

struct CognateRule {
    std::string en;
    std::string es;
    std::vector<std::pair<std::string, std::string>> examples;  // English, Spanish
};

struct FalseFriend {
    std::string es;
    std::string meaning;
    std::string looksLike;
    std::string realWord;  // the Spanish for the English look-alike
};

struct GenderRule {
    std::string rule;
    std::string examples;
    std::string note;
};

struct GenderWord {
    std::string es;  // with its article
    std::string en;
    std::string note;
};

struct PronunciationGroup {
    std::string title;
    std::string tip;
    std::vector<std::string> items;
};

struct DescribeItem {
    std::string en;
    std::string es;
    std::string model;
};

struct MockQuestion {
    std::string question;
    std::vector<std::string> options;  // the first is correct
};

struct MockPart {
    std::string text;
    std::vector<MockQuestion> questions;
};

struct SpeakingTask {
    std::string title;
    std::string prompt;
    std::string image;
};

struct MockExam {
    MockPart reading;
    MockPart listening;
    std::string writingPrompt;
    std::vector<SpeakingTask> speaking;
};

// The Immersion screen: listening to pick from, how to watch a series, devices to switch to Spanish and
// phrases for a language exchange.
struct Resource {
    std::string name;
    std::string level;   // "A2+ → B2"
    std::string accent;
    std::string kind;    // "Podcast", "Video"…
    std::string url;
    std::string note;
};

// A series made in Spanish, for the level it suits (lq::seriesLevel picks the learner's).
struct Series {
    struct Where {
        std::string service;  // "Netflix"
        std::string detail;   // "complete", "free with ads", or empty
    };
    std::string name;
    std::string level;  // "A1", "A2" or "B1"
    std::string years;
    std::string kind;   // "Sitcom"…
    std::string note;
    std::vector<Where> where;
};

struct Mission {
    std::string id;  // the web app's, so a web backup keeps its ticks
    std::string title;
    std::string how;
};

struct Immersion {
    std::vector<Resource> resources;
    std::vector<Series> series;
    std::string seriesWhere;  // which country's catalogue, and when
    std::vector<std::string> seriesTips;
    std::vector<Mission> missions;
    std::vector<std::pair<std::string, std::string>> exchangePhrases;  // Spanish, English
};

struct Course {
    std::vector<Phase> phases;
    std::vector<Unit> units;
    std::map<std::string, std::string, std::less<>> pastReasons;
    std::vector<PastItem> past;
    std::vector<CognateRule> cognateRules;
    std::vector<FalseFriend> falseFriends;
    std::vector<GenderRule> genderRules;
    std::vector<GenderWord> genderWords;
    std::vector<PronunciationGroup> pronunciation;
    std::vector<std::string> describeFrames;
    std::vector<DescribeItem> describe;
    MockExam mock;
    Immersion immersion;

    // A vocabulary id where it first appears in the course, with its unit; nullptr when unknown.
    const Vocab *word(std::string_view id, int *unit = nullptr) const;
    // Every word once, in course order (the web app's vocabIndex).
    std::vector<std::pair<int, const Vocab *>> allWords() const;
    // The vocabulary ids of each unit, which the demo learner and learnUnit need.
    std::vector<std::vector<std::string>> unitWordIds() const;

    std::map<std::string, std::pair<int, std::size_t>, std::less<>> index;  // id → unit, position
};

// Parses course.json. On failure returns nullopt and says why in error.
std::optional<Course> parseCourse(std::string_view json, std::string *error);

} // namespace lq
