#pragma once

#include "lq/course.h"
#include "lq/exercises.h"
#include "lq/random.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// The A1, A2 and B1 level assessments behind the Certificates of Readiness (desktop/content/assessments.json).
// Each one is marked by the app alone, with no self-assessment: reading, listening, and grammar and vocabulary.
// It opens once the level's lessons are done, and passes with 70% overall and at least 60% in every part.
namespace lq {

// A text to read or to listen to, with its questions (in each, the first option is right).
struct AssessmentPassage {
    std::string intro;  // listening: what the learner is about to hear, in English
    std::string text;
    std::vector<MockQuestion> questions;
};

struct LevelAssessment {
    std::string level;  // "A1", "A2" or "B1"
    std::vector<AssessmentPassage> reading;
    std::vector<AssessmentPassage> listening;
    std::vector<MockQuestion> language;  // grammar and vocabulary
};

struct Assessments {
    std::vector<LevelAssessment> levels;
    const LevelAssessment *level(std::string_view level) const;
};

// What one attempt asks: one reading text, one listening text and kAssessmentLanguageItems of the language
// questions, all questions of each text.
inline constexpr int kAssessmentPassageQuestions = 5;
inline constexpr int kAssessmentLanguageItems = 12;
inline constexpr int kAssessmentPassPercent = 70;
inline constexpr int kAssessmentPartPercent = 60;

// Parses assessments.json and checks it: A1, A2 and B1, each with two texts or more per part, five questions to
// a text, at least kAssessmentLanguageItems language questions and three distinct options to every question.
std::optional<Assessments> parseAssessments(std::string_view json, std::string *error);

// The lessons a level's assessment needs: the first five units for A1 (phase 1), ten for A2 (phases 1–2) and all
// 25 for B1. 0 for an unknown level.
int assessmentUnits(std::string_view level);

// One attempt: which texts and questions it asks, drawn at random.
struct AssessmentDraw {
    std::string level;
    int reading = -1;
    int listening = -1;
    std::vector<int> language;
    bool valid() const { return reading >= 0 && listening >= 0 && !language.empty(); }
};

AssessmentDraw drawAssessment(const LevelAssessment &assessment, SeededRandom &rnd);

// The questions of one part of a draw ("reading", "listening" or "language"), as choice items with the options
// shuffled. Empty for an unknown part or a draw that does not fit the assessment.
std::vector<Item> assessmentItems(const LevelAssessment &assessment, const AssessmentDraw &draw, std::string_view part,
                                  SeededRandom &rnd);

struct AssessmentScores {
    Tally reading;
    Tally listening;
    Tally language;
};

struct AssessmentResult {
    int reading = 0;  // percentages, rounded down
    int listening = 0;
    int language = 0;
    int overall = 0;
    bool complete = false;      // every part has the number of questions an attempt asks
    bool overallPassed = false;  // 70% or more of all the questions
    bool partsPassed = false;    // 60% or more in every part
    bool pass() const { return complete && overallPassed && partsPassed; }
};

// Marks an attempt. The thresholds are compared on whole answers (right × 100 ≥ asked × percent), never on
// rounded percentages, so 69.5% does not pass.
AssessmentResult assessmentResult(const AssessmentScores &scores);

} // namespace lq
