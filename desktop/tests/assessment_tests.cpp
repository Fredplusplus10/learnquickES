#include "lq/assessment.h"
#include "lq/course.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <set>
#include <string>

namespace {

const lq::Assessments &assessments()
{
    static const lq::Assessments parsed = [] {
        std::string error;
        auto a = lq::parseAssessments(lq::test::readFile(LQ_CONTENT_DIR "/assessments.json"), &error);
        INFO(error);
        REQUIRE(a.has_value());
        return *a;
    }();
    return parsed;
}

lq::AssessmentScores scores(int reading, int listening, int language)
{
    return {{reading, lq::kAssessmentPassageQuestions},
            {listening, lq::kAssessmentPassageQuestions},
            {language, lq::kAssessmentLanguageItems}};
}

} // namespace

TEST_CASE("the bundled assessments cover A1, A2 and B1 with enough to draw from", "[assessment]")
{
    const lq::Assessments &a = assessments();
    REQUIRE(a.levels.size() == 3);
    std::set<std::string> texts;
    for (const lq::LevelAssessment &level : a.levels) {
        INFO(level.level);
        CHECK(level.reading.size() >= 2);
        CHECK(level.listening.size() >= 2);
        CHECK(level.language.size() >= 20);
        for (const auto *part : {&level.reading, &level.listening}) {
            for (const lq::AssessmentPassage &p : *part) {
                CHECK(p.questions.size() == static_cast<std::size_t>(lq::kAssessmentPassageQuestions));
                CHECK(texts.insert(p.text).second);  // no text is used twice, in any level
            }
        }
        for (const lq::AssessmentPassage &p : level.listening)
            CHECK_FALSE(p.intro.empty());
        // A fill-in line has its gap, so the learner sees what to fill.
        for (const lq::MockQuestion &q : level.language)
            CHECK((q.question.find("___") != std::string::npos));
    }
    CHECK(a.level("B1") == &a.levels[2]);
    CHECK(a.level("C1") == nullptr);
}

TEST_CASE("an assessment never asks a question from the lessons or the B1 mock", "[assessment]")
{
    const auto course = nlohmann::json::parse(lq::test::readFile(LQ_CONTENT_DIR "/course.json"));
    std::set<std::string> practised;
    for (const auto &unit : course.at("units")) {
        for (const auto &ex : unit.at("ex"))
            practised.insert(ex.at("q").get<std::string>());
    }
    for (const char *part : {"reading", "listening"}) {
        for (const auto &q : course.at("mock").at(part).at("questions"))
            practised.insert(q.at("q").get<std::string>());
    }
    for (const lq::LevelAssessment &level : assessments().levels) {
        for (const lq::MockQuestion &q : level.language) {
            INFO(level.level << ": " << q.question);
            CHECK_FALSE(practised.contains(q.question));
        }
        for (const auto *part : {&level.reading, &level.listening}) {
            for (const lq::AssessmentPassage &p : *part) {
                for (const lq::MockQuestion &q : p.questions) {
                    INFO(level.level << ": " << q.question);
                    CHECK_FALSE(practised.contains(q.question));
                }
            }
        }
    }
}

TEST_CASE("each level's lessons are the phases it covers", "[assessment]")
{
    CHECK(lq::assessmentUnits("A1") == 5);
    CHECK(lq::assessmentUnits("A2") == 10);
    CHECK(lq::assessmentUnits("B1") == 25);
    CHECK(lq::assessmentUnits("C1") == 0);
    // Units are in phase order, five to a phase, so "the first N" is "these phases".
    const auto course = nlohmann::json::parse(lq::test::readFile(LQ_CONTENT_DIR "/course.json"));
    int index = 0;
    for (const auto &unit : course.at("units")) {
        CHECK(unit.at("phase").get<int>() == index / 5 + 1);
        ++index;
    }
}

TEST_CASE("a draw takes one text per part and 12 distinct language questions, the same for the same seed", "[assessment]")
{
    const lq::LevelAssessment &a2 = *assessments().level("A2");
    lq::SeededRandom one("profile/A2/2026-11-09");
    lq::SeededRandom two("profile/A2/2026-11-09");
    const lq::AssessmentDraw first = lq::drawAssessment(a2, one);
    const lq::AssessmentDraw again = lq::drawAssessment(a2, two);
    REQUIRE(first.valid());
    CHECK(first.level == "A2");
    CHECK(first.reading == again.reading);
    CHECK(first.listening == again.listening);
    CHECK(first.language == again.language);
    CHECK(first.language.size() == static_cast<std::size_t>(lq::kAssessmentLanguageItems));
    CHECK(std::set<int>(first.language.begin(), first.language.end()).size() == first.language.size());

    // Over many days every text and question comes up.
    std::set<int> readings, listenings, language;
    for (int day = 1; day <= 60; ++day) {
        lq::SeededRandom rnd("profile/A2/" + std::to_string(day));
        const lq::AssessmentDraw d = lq::drawAssessment(a2, rnd);
        readings.insert(d.reading);
        listenings.insert(d.listening);
        language.insert(d.language.begin(), d.language.end());
    }
    CHECK(readings.size() == a2.reading.size());
    CHECK(listenings.size() == a2.listening.size());
    CHECK(language.size() == a2.language.size());
}

TEST_CASE("a draw's items are choices whose right option is the content's first", "[assessment]")
{
    const lq::LevelAssessment &b1 = *assessments().level("B1");
    lq::SeededRandom rnd(42u);
    const lq::AssessmentDraw draw = lq::drawAssessment(b1, rnd);
    const auto reading = lq::assessmentItems(b1, draw, "reading", rnd);
    const auto listening = lq::assessmentItems(b1, draw, "listening", rnd);
    const auto language = lq::assessmentItems(b1, draw, "language", rnd);
    REQUIRE(reading.size() == 5);
    REQUIRE(listening.size() == 5);
    REQUIRE(language.size() == 12);
    for (std::size_t i = 0; i < reading.size(); ++i) {
        const lq::MockQuestion &q = b1.reading[static_cast<std::size_t>(draw.reading)].questions[i];
        CHECK(reading[i].type == lq::ItemType::Choice);
        CHECK(reading[i].skill == "reading");
        CHECK(reading[i].options.at(static_cast<std::size_t>(reading[i].answer)) == q.options.front());
    }
    for (std::size_t i = 0; i < language.size(); ++i) {
        const lq::MockQuestion &q = b1.language[static_cast<std::size_t>(draw.language[i])];
        CHECK(language[i].question == q.question);
        CHECK(language[i].options.at(static_cast<std::size_t>(language[i].answer)) == q.options.front());
    }
    CHECK(listening.front().skill == "listening");
    CHECK(language.front().skill == "grammar");
    CHECK(lq::assessmentItems(b1, draw, "speaking", rnd).empty());
    // A draw made for another level asks nothing.
    CHECK(lq::assessmentItems(*assessments().level("A1"), draw, "reading", rnd).empty());
}

TEST_CASE("an attempt passes with 70% overall and 60% in every part, counted on whole answers", "[assessment]")
{
    CHECK(lq::assessmentResult(scores(5, 5, 12)).pass());
    CHECK(lq::assessmentResult(scores(5, 5, 12)).overall == 100);
    // 16 of 22 (72.7%) with every part at 60% or more passes; 15 of 22 (68.2%) does not.
    CHECK(lq::assessmentResult(scores(3, 3, 10)).pass());
    const lq::AssessmentResult fifteen = lq::assessmentResult(scores(3, 3, 9));
    CHECK_FALSE(fifteen.pass());
    CHECK_FALSE(fifteen.overallPassed);
    CHECK(fifteen.partsPassed);
    CHECK(fifteen.overall == 68);
    // A strong overall score does not make up for one weak part.
    const lq::AssessmentResult weakListening = lq::assessmentResult(scores(5, 2, 12));
    CHECK(weakListening.overallPassed);
    CHECK_FALSE(weakListening.partsPassed);
    CHECK_FALSE(weakListening.pass());
    CHECK(weakListening.listening == 40);
    CHECK_FALSE(lq::assessmentResult(scores(5, 5, 7)).pass());  // 58% in language
    CHECK(lq::assessmentResult(scores(4, 4, 8)).pass());        // 16 of 22, 66.7% in language

    // An attempt that asked less than a full one is never a pass.
    const lq::AssessmentResult partial = lq::assessmentResult({{5, 5}, {5, 5}, {6, 6}});
    CHECK_FALSE(partial.complete);
    CHECK_FALSE(partial.pass());
    CHECK_FALSE(lq::assessmentResult({{6, 5}, {5, 5}, {12, 12}}).pass());
    CHECK_FALSE(lq::assessmentResult({}).pass());
}

TEST_CASE("damaged assessment files are refused with a reason", "[assessment]")
{
    std::string error;
    CHECK_FALSE(lq::parseAssessments("{", &error));
    CHECK_FALSE(error.empty());
    CHECK_FALSE(lq::parseAssessments(R"({"levels": []})", &error));
    CHECK(error.find("A1, A2 and B1") != std::string::npos);

    // The bundled file with one option removed from one question.
    auto doc = nlohmann::json::parse(lq::test::readFile(LQ_CONTENT_DIR "/assessments.json"));
    doc["levels"][1]["language"][0]["o"].erase(2);
    CHECK_FALSE(lq::parseAssessments(doc.dump(), &error));
    CHECK(error.find("A2") != std::string::npos);
    doc = nlohmann::json::parse(lq::test::readFile(LQ_CONTENT_DIR "/assessments.json"));
    doc["levels"][2]["reading"][0]["questions"].erase(0);
    CHECK_FALSE(lq::parseAssessments(doc.dump(), &error));
    CHECK(error.find("five questions") != std::string::npos);
    doc = nlohmann::json::parse(lq::test::readFile(LQ_CONTENT_DIR "/assessments.json"));
    doc["levels"][0]["language"][1] = doc["levels"][0]["language"][0];
    CHECK_FALSE(lq::parseAssessments(doc.dump(), &error));
    CHECK(error.find("asked twice") != std::string::npos);
}
