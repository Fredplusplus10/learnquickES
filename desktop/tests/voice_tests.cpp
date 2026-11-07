// The bundled neural voice, on the model downloaded at configure time.
#include "lq/speech.h"
#include "lq/supertonic.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

namespace {

const lq::SupertonicVoice &voice()
{
    static const std::unique_ptr<lq::SupertonicVoice> loaded = [] {
        std::string error;
        auto v = lq::SupertonicVoice::load(LQ_VOICE_DIR, &error);
        if (!v)
            throw std::runtime_error(error);
        return v;
    }();
    return *loaded;
}

double seconds(const std::vector<std::int16_t> &pcm)
{
    return static_cast<double>(pcm.size()) / voice().sampleRate();
}

int peak(const std::vector<std::int16_t> &pcm)
{
    int most = 0;
    for (const std::int16_t s : pcm)
        most = std::max(most, std::abs(static_cast<int>(s)));
    return most;
}

// How loud the speech is, in dB below full scale: the 10 ms frames above 2 % of the peak, as the voice levels it.
double speechLevel(const std::vector<std::int16_t> &pcm)
{
    const auto frame = static_cast<std::size_t>(voice().sampleRate() / 100);
    const double loudest = peak(pcm);
    double sum = 0;
    std::size_t count = 0;
    for (std::size_t start = 0; start + frame <= pcm.size(); start += frame) {
        int most = 0;
        for (std::size_t i = start; i < start + frame; ++i)
            most = std::max(most, std::abs(static_cast<int>(pcm[i])));
        if (most <= 0.02 * loudest)
            continue;
        for (std::size_t i = start; i < start + frame; ++i)
            sum += static_cast<double>(pcm[i]) * pcm[i];
        count += frame;
    }
    return count > 0 ? 20.0 * std::log10(std::sqrt(sum / static_cast<double>(count)) / 32767.0) : -120.0;
}

// Everything the app reads aloud: the keys the lesson and exercise screens speak, and the dialogue lines.
std::vector<std::string> spokenLines()
{
    using nlohmann::json;
    static const std::set<std::string> keys{"es",      "example", "phrases",  "model",    "audio",
                                            "answer",  "answers", "lines",    "sentence", "listeningText"};
    std::vector<std::string> lines;
    const auto walk = [&](const auto &self, const json &value, const std::string &key) -> void {
        if (value.is_object()) {
            for (const auto &[k, v] : value.items())
                self(self, v, k);
        } else if (value.is_array() && key == "dialogue") {
            for (const json &line : value)
                lines.push_back(line.at(1).get<std::string>());
        } else if (value.is_array()) {
            for (const json &item : value)
                self(self, item, key);
        } else if (value.is_string() && keys.contains(key)) {
            lines.push_back(value.get<std::string>());
        }
    };
    walk(walk, json::parse(lq::test::readFile(LQ_CONTENT_DIR "/course.json")), {});
    // The level assessments' listening texts, read aloud by the same voice.
    for (const json &level : json::parse(lq::test::readFile(LQ_CONTENT_DIR "/assessments.json")).at("levels")) {
        for (const json &passage : level.at("listening"))
            lines.push_back(passage.at("text").get<std::string>());
    }
    return lines;
}

} // namespace

TEST_CASE("the voice has every style the app speaks with", "[voice]")
{
    REQUIRE(voice().sampleRate() == 44100);
    CHECK(voice().hasStyle(lq::kNarratorVoice));
    using nlohmann::json;
    const json course = json::parse(lq::test::readFile(LQ_CONTENT_DIR "/course.json"));
    for (const json &unit : course.at("units")) {
        for (const json &line : unit.at("dialogue")) {
            INFO(line.at(0).get<std::string>());
            CHECK(voice().hasStyle(lq::voiceFor(line.at(0).get<std::string>())));
        }
    }
    CHECK_FALSE(voice().hasStyle("X1"));
    // Every voice Settings offers ships with the app.
    for (const lq::VoiceChoice &choice : lq::voiceChoices()) {
        INFO(choice.style);
        CHECK(voice().hasStyle(choice.style));
    }
    CHECK(lq::voiceChoices().front().style == lq::kNarratorVoice);
}

TEST_CASE("the voice speaks a phrase at a steady loudness, the same way every time", "[voice]")
{
    const std::string text = lq::speechText("Hola, ¿cómo estás?");
    for (const char *style : {"F3", "M4"}) {
        INFO(style);
        const auto pcm = voice().synthesize(text, style);
        CHECK(seconds(pcm) > 0.5);
        CHECK(seconds(pcm) < 4.0);
        // Its speech at -18 dBFS in every voice, with no peak above -1 dBFS.
        CHECK(speechLevel(pcm) > -18.5);
        CHECK(speechLevel(pcm) < -17.5);
        CHECK(peak(pcm) <= 29163);
        CHECK(voice().synthesize(text, style) == pcm);
    }
    CHECK(voice().synthesize(text, "M5") != voice().synthesize(text, "M1"));
}

TEST_CASE("every pace Settings offers lasts as long as it says, and sentences are spoken apart", "[voice]")
{
    const std::string text = lq::speechText("Me gusta el café.");
    const double natural = seconds(voice().synthesize(text, "F3"));
    // 0.85: a voice slower than the pace asked speaks quicker than its own pace.
    for (const double pace : {0.85, lq::kDefaultPace, lq::kDefaultSlowPace, lq::kMaxPace, lq::kMaxSlowPace}) {
        INFO(pace);
        const auto pcm = voice().synthesize(text, "F3", static_cast<float>(pace));
        CHECK(seconds(pcm) > natural * (pace - 0.05));
        CHECK(seconds(pcm) < natural * (pace + 0.05));
        CHECK(speechLevel(pcm) > -19.0);
        CHECK(speechLevel(pcm) < -17.5);
    }

    std::string paragraph;
    while (paragraph.size() < 400)
        paragraph += "Ayer fui al mercado y compré frutas y verduras. ";
    const auto longestSilence = [](const std::vector<std::int16_t> &pcm) {
        std::size_t run = 0;
        std::size_t longest = 0;
        for (const std::int16_t s : pcm) {
            run = s == 0 ? run + 1 : 0;
            longest = std::max(longest, run);
        }
        return static_cast<double>(longest) / voice().sampleRate();
    };
    // The pauses between sentences are drawn out to the length asked: silence is added to the quiet the model
    // leaves there, so the longest run of pure silence is a little shorter than the pause, never longer.
    const double unpaused = seconds(voice().synthesize(paragraph, "F3", 1.0f, 0.0f));
    for (const double pause : {lq::kSentencePause, lq::kSlowSentencePause}) {
        INFO(pause);
        const auto pcm = voice().synthesize(paragraph, "F3", 1.0f, static_cast<float>(pause));
        CHECK(seconds(pcm) > 15.0);
        CHECK(seconds(pcm) > unpaused);
        CHECK(longestSilence(pcm) > pause - 0.5);
        CHECK(longestSilence(pcm) < pause + 0.05);
    }
}

TEST_CASE("every voice on offer speaks at the same pace", "[voice]")
{
    // Their own paces run from 4.0 to 5.2 syllables a second; brought to one pace, the same lines last about as
    // long in every voice (within 3 % when measured).
    const std::string text = lq::speechText("¡Hola! Soy Lucía. ¿Y tú? Desde hace tres días. Anoche también tuve "
                                            "fiebre. Ayer fui al mercado y compré frutas y verduras.");
    const auto at = [&](std::string_view style) {
        return seconds(voice().synthesize(text, style, static_cast<float>(lq::voiceStretch(style, lq::kDefaultPace))));
    };
    const double narrator = at(lq::kNarratorVoice);
    for (const lq::VoiceChoice &choice : lq::voiceChoices()) {
        INFO(choice.style);
        CHECK(at(choice.style) > narrator * 0.9);
        CHECK(at(choice.style) < narrator * 1.1);
    }
}

TEST_CASE("nothing to say, or a voice that does not exist, gives no audio", "[voice]")
{
    CHECK(voice().synthesize("", "F3").empty());
    CHECK(voice().synthesize(lq::speechText(" … "), "F3").empty());
    CHECK(voice().synthesize("Hola.", "X1").empty());
}

TEST_CASE("the text reaches the model as Supertone's own code prepares it", "[voice]")
{
    // From the supertonic Python package (1.3.1): <es>¿Qué tal?</es>, with é split into e and U+0301.
    CHECK(voice().textIds("¿Qué tal?")
          == std::vector<std::int64_t>{27, 60, 74, 28, 87, 46, 76, 60, 90, 0, 75, 56, 67, 29, 27, 14, 60, 74, 28});
    // Curly quotes become straight ones, spaces collapse, and the closing quote counts as an ending.
    CHECK(voice().textIds("Él dijo  “sí” ")
          == std::vector<std::int64_t>{27, 60, 74, 28, 34, 90, 67, 0,  59, 64, 65, 70,
                                       0,  2,  74, 64, 90, 2,  27, 14, 60, 74, 28});
}

TEST_CASE("the voice can say every line of the course", "[voice]")
{
    const std::vector<std::string> lines = spokenLines();
    REQUIRE(lines.size() > 500);
    for (const std::string &line : lines) {
        INFO(line);
        CHECK(voice().unknownCharacters(lq::speechText(line)).empty());
    }
}
