#include "lq/assessment.h"

#include "lq/calendar.h"

#include <nlohmann/json.hpp>

#include <numeric>
#include <set>

namespace lq {

namespace {

using nlohmann::json;

const std::vector<std::string> kLevels{"A1", "A2", "B1"};

MockQuestion parseQuestion(const json &q)
{
    return {q.at("q").get<std::string>(), q.at("o").get<std::vector<std::string>>()};
}

AssessmentPassage parsePassage(const json &p)
{
    AssessmentPassage passage;
    passage.intro = p.value("intro", "");
    passage.text = p.at("text").get<std::string>();
    for (const json &q : p.at("questions"))
        passage.questions.push_back(parseQuestion(q));
    return passage;
}

// Why a question cannot be asked, or "" when it can.
std::string questionProblem(const MockQuestion &q)
{
    if (q.question.empty())
        return "a question has no text";
    if (q.options.size() != 3)
        return "\"" + q.question + "\" needs three options";
    if (std::set<std::string>(q.options.begin(), q.options.end()).size() != q.options.size())
        return "\"" + q.question + "\" repeats an option";
    for (const std::string &option : q.options) {
        if (option.empty())
            return "\"" + q.question + "\" has an empty option";
    }
    return {};
}

std::string passageProblem(const std::vector<AssessmentPassage> &passages, const char *part)
{
    if (passages.size() < 2)
        return std::string("two ") + part + " texts or more are needed";
    for (const AssessmentPassage &p : passages) {
        if (p.text.empty())
            return std::string("a ") + part + " text is empty";
        if (p.questions.size() != static_cast<std::size_t>(kAssessmentPassageQuestions))
            return std::string("every ") + part + " text needs five questions";
        for (const MockQuestion &q : p.questions) {
            if (std::string problem = questionProblem(q); !problem.empty())
                return problem;
        }
    }
    return {};
}

int percentOf(const Tally &t)
{
    return t.total > 0 ? t.correct * 100 / t.total : 0;
}

bool reaches(const Tally &t, int percent)
{
    return t.total > 0 && t.correct * 100 >= t.total * percent;
}

bool fits(const Tally &t, int asked)
{
    return t.total == asked && t.correct >= 0 && t.correct <= t.total;
}

std::vector<Item> choiceItems(const std::vector<MockQuestion> &questions, const std::string &skill, SeededRandom &rnd)
{
    std::vector<Item> items;
    for (const MockQuestion &q : questions) {
        Item item = choiceItem(q.question, q.options, rnd);
        item.skill = skill;
        items.push_back(std::move(item));
    }
    return items;
}

} // namespace

const LevelAssessment *Assessments::level(std::string_view level) const
{
    for (const LevelAssessment &a : levels) {
        if (a.level == level)
            return &a;
    }
    return nullptr;
}

std::optional<Assessments> parseAssessments(std::string_view text, std::string *error)
{
    const auto fail = [error](std::string why) -> std::optional<Assessments> {
        if (error)
            *error = std::move(why);
        return std::nullopt;
    };
    Assessments out;
    try {
        const json doc = json::parse(text);
        for (const json &l : doc.at("levels")) {
            LevelAssessment a;
            a.level = l.at("level").get<std::string>();
            for (const json &p : l.at("reading"))
                a.reading.push_back(parsePassage(p));
            for (const json &p : l.at("listening"))
                a.listening.push_back(parsePassage(p));
            for (const json &q : l.at("language"))
                a.language.push_back(parseQuestion(q));
            out.levels.push_back(std::move(a));
        }
    } catch (const json::exception &e) {
        return fail(e.what());
    }
    if (out.levels.size() != kLevels.size())
        return fail("the assessments should be A1, A2 and B1");
    for (std::size_t i = 0; i < kLevels.size(); ++i) {
        const LevelAssessment &a = out.levels[i];
        if (a.level != kLevels[i])
            return fail("the assessments should be A1, A2 and B1, in that order");
        if (std::string problem = passageProblem(a.reading, "reading"); !problem.empty())
            return fail(a.level + ": " + problem);
        if (std::string problem = passageProblem(a.listening, "listening"); !problem.empty())
            return fail(a.level + ": " + problem);
        if (a.language.size() < static_cast<std::size_t>(kAssessmentLanguageItems))
            return fail(a.level + ": at least 12 language questions are needed");
        std::set<std::string> asked;
        for (const MockQuestion &q : a.language) {
            if (std::string problem = questionProblem(q); !problem.empty())
                return fail(a.level + ": " + problem);
            if (!asked.insert(q.question).second)
                return fail(a.level + ": \"" + q.question + "\" is asked twice");
        }
    }
    return out;
}

int assessmentUnits(std::string_view level)
{
    if (level == "A1")
        return kUnitsPerPhase;
    if (level == "A2")
        return 2 * kUnitsPerPhase;
    if (level == "B1")
        return kPhases * kUnitsPerPhase;
    return 0;
}

AssessmentDraw drawAssessment(const LevelAssessment &assessment, SeededRandom &rnd)
{
    AssessmentDraw draw;
    draw.level = assessment.level;
    if (assessment.reading.empty() || assessment.listening.empty() || assessment.language.empty())
        return draw;
    draw.reading = static_cast<int>(rnd.next() * static_cast<double>(assessment.reading.size()));
    draw.listening = static_cast<int>(rnd.next() * static_cast<double>(assessment.listening.size()));
    std::vector<int> all(assessment.language.size());
    std::iota(all.begin(), all.end(), 0);
    draw.language = sampled(std::move(all), static_cast<std::size_t>(kAssessmentLanguageItems), rnd);
    return draw;
}

std::vector<Item> assessmentItems(const LevelAssessment &assessment, const AssessmentDraw &draw, std::string_view part,
                                  SeededRandom &rnd)
{
    if (!draw.valid() || draw.level != assessment.level)
        return {};
    const auto inRange = [](int index, std::size_t size) { return index >= 0 && static_cast<std::size_t>(index) < size; };
    if (part == "reading") {
        if (!inRange(draw.reading, assessment.reading.size()))
            return {};
        return choiceItems(assessment.reading[static_cast<std::size_t>(draw.reading)].questions, "reading", rnd);
    }
    if (part == "listening") {
        if (!inRange(draw.listening, assessment.listening.size()))
            return {};
        return choiceItems(assessment.listening[static_cast<std::size_t>(draw.listening)].questions, "listening", rnd);
    }
    if (part == "language") {
        std::vector<MockQuestion> questions;
        for (const int index : draw.language) {
            if (!inRange(index, assessment.language.size()))
                return {};
            questions.push_back(assessment.language[static_cast<std::size_t>(index)]);
        }
        return choiceItems(questions, "grammar", rnd);
    }
    return {};
}

AssessmentResult assessmentResult(const AssessmentScores &s)
{
    AssessmentResult r;
    r.reading = percentOf(s.reading);
    r.listening = percentOf(s.listening);
    r.language = percentOf(s.language);
    const Tally all{s.reading.correct + s.listening.correct + s.language.correct,
                    s.reading.total + s.listening.total + s.language.total};
    r.overall = percentOf(all);
    r.complete = fits(s.reading, kAssessmentPassageQuestions) && fits(s.listening, kAssessmentPassageQuestions)
                 && fits(s.language, kAssessmentLanguageItems);
    r.overallPassed = reaches(all, kAssessmentPassPercent);
    r.partsPassed = reaches(s.reading, kAssessmentPartPercent) && reaches(s.listening, kAssessmentPartPercent)
                    && reaches(s.language, kAssessmentPartPercent);
    return r;
}

} // namespace lq
