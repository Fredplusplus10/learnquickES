// What the voice reads: symbols turned into words, and the phonemes of the bundled neural voice checked
// against espeak-ng, whose phonemes the voice was trained on (data/phonemes_es419.json, written by
// desktop/tools/phoneme_reference.py).
#include "lq/speech.h"
#include "test_support.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <cmath>
#include <set>
#include <string>

using lq::phonemizeClause;
using lq::phonemizeSentences;
using lq::speechText;

TEST_CASE("symbols are read as words, never as their names", "[speech]")
{
    CHECK(speechText("cansado / cansada") == "cansado o cansada");
    CHECK(speechText("contento/contenta") == "contento o contenta");
    CHECK(speechText("él/ella/usted") == "él, ella o usted");
    CHECK(speechText("siete / ocho") == "siete u ocho");
    CHECK(speechText("uno/hoja") == "uno u hoja");
    CHECK(speechText("tener que + infinitivo") == "tener que más infinitivo");
    CHECK(speechText("cerca (de)") == "cerca de");
    CHECK(speechText("más … que") == "más, que");
    CHECK(speechText("tan ... como") == "tan, como");
    CHECK(speechText("Fundamentos 80/20") == "Fundamentos ochenta veinte");
    CHECK(speechText("Yo ___ café.") == "Yo, café.");
    CHECK(speechText("  Hola,   ¿qué tal?  ") == "Hola, ¿qué tal?");
}

TEST_CASE("numbers are read as words, and names as they are said", "[speech]")
{
    CHECK(speechText("Aquí tiene una llave nueva para la habitación 305.")
          == "Aquí tiene una llave nueva para la habitación trescientos cinco.");
    CHECK(speechText("Tengo 35 años.") == "Tengo treinta y cinco años.");
    CHECK(speechText("En 2019 fui a México.") == "En dos mil diecinueve fui a México.");
    CHECK(speechText("de 2015 a 2020") == "de dos mil quince a dos mil veinte");
    CHECK(speechText("1.000") == "mil");
    CHECK(speechText("21.000 y 31.000") == "veintiún mil y treinta y un mil");
    CHECK(speechText("100, 150, 500, 700") == "cien, ciento cincuenta, quinientos, setecientos");
    CHECK(speechText("El jueves a las 4:30") == "El jueves a las cuatro y media");
    CHECK(speechText("a las 10:00") == "a las diez");
    CHECK(speechText("a las 9:15 o a las 9:40") == "a las nueve y cuarto o a las nueve y cuarenta");
    CHECK(speechText("Español B1 real") == "Español B uno real");
    CHECK(speechText("Soy de Ohio.") == "Soy de Ojáio.");
    CHECK(speechText("Ohiolandia") == "Ohiolandia");  // only the whole word
}

TEST_CASE("every voice is brought to the same pace", "[speech]")
{
    for (const lq::VoiceChoice &choice : lq::voiceChoices()) {
        INFO(choice.style);
        const double own = lq::voiceSyllablesPerSecond(choice.style);
        REQUIRE(own > 3.5);
        for (const double pace : {lq::kMinPace, lq::kDefaultPace, lq::kDefaultSlowPace, lq::kMaxSlowPace}) {
            const double heard = own / lq::voiceStretch(choice.style, pace);
            CHECK(std::abs(heard - lq::kReferenceSyllablesPerSecond / pace) < 1e-9);
        }
    }
    CHECK(lq::voiceStretch("M5", lq::kMinPace) < 1.0);  // the slowest voice speaks quicker than its own pace
    CHECK(lq::voiceSyllablesPerSecond("X9") == 0.0);
    CHECK(lq::voiceStretch("X9", 1.3) == 1.0);
}

TEST_CASE("every dialogue speaker has a voice of their own", "[speech]")
{
    CHECK(lq::voiceFor("Lucía") == "F3");
    CHECK(lq::voiceFor("Tú") == "M2");
    CHECK(lq::voiceFor("Nadie").empty());

    // Women speak with female voices and men with male ones; nobody but the learner sounds like the learner.
    const std::set<std::string> women{"Lucía", "Ana", "Sofía", "Marta", "Elena", "Valeria", "Laura", "Isabel",
                                      "Amiga", "Jefa", "Señora", "Vendedora", "Profesora", "Doctora",
                                      "Entrevistadora", "Recepcionista"};
    using nlohmann::json;
    const json course = json::parse(lq::test::readFile(LQ_CONTENT_DIR "/course.json"));
    for (const json &unit : course.at("units")) {
        for (const json &line : unit.at("dialogue")) {
            const auto speaker = line.at(0).get<std::string>();
            INFO(speaker);
            const std::string_view voice = lq::voiceFor(speaker);
            REQUIRE(voice.size() == 2);
            if (speaker == "Tú")
                continue;
            CHECK(voice[0] == (women.contains(speaker) ? 'F' : 'M'));
            CHECK(voice != lq::voiceFor("Tú"));
        }
    }
}

TEST_CASE("every clause the app reads is phonemized as espeak-ng does", "[speech]")
{
    using nlohmann::json;
    const json data = json::parse(lq::test::readFile(LQ_TEST_DATA_DIR "/phonemes_es419.json"));
    REQUIRE(data.at("voice") == "es-419");
    // Known differences, all outside the course's spoken text: a spelled abbreviation, one word where
    // espeak-ng keeps d as a stop after a vowel, and an English loanword.
    const std::set<std::string> known{"Fui vs", "Me duele mucho la garganta", "whisky"};
    int compared = 0;
    for (const auto &[text, want] : data.at("clauses").items()) {
        if (known.contains(text))
            continue;
        CAPTURE(text);
        CHECK(phonemizeClause(text) == want.get<std::string>());
        ++compared;
    }
    CHECK(compared > 500);
}

TEST_CASE("phonemes follow the rules of Latin-American Spanish", "[speech]")
{
    CHECK(phonemizeClause("hola") == "ˈola");
    CHECK(phonemizeClause("la ciudad") == "la sjuðˈad");      // seseo, final d
    CHECK(phonemizeClause("un viaje") == "ˈum bjˈaxe");       // n before b
    CHECK(phonemizeClause("la calle") == "la kˈajje");
    CHECK(phonemizeClause("el perro") == "el pˈero");         // trilled rr
    CHECK(phonemizeClause("el gato") == "el ɣˈato");          // fricative g after a vowel sound
    CHECK(phonemizeClause("cerca de") == "sˈeɾka ðˈe");       // a clause-final "de" is stressed
    CHECK(phonemizeClause("rápidamente") == "rˈapiðamˈente"); // -mente keeps both stresses
    CHECK(phonemizeClause("20") == "bˈeɪnte");
    CHECK(phonemizeClause("") == "");
}

TEST_CASE("sentences carry the punctuation the voice was trained with", "[speech]")
{
    const auto sentences = phonemizeSentences(speechText("Hola, ¿qué tal? Bien."));
    REQUIRE(sentences.size() == 2);
    CHECK(sentences[0] == U"ˈola, kˈe tˈal?");
    CHECK(sentences[1] == U"bjˈen.");
    CHECK(phonemizeSentences("cansado o cansada") == std::vector<std::u32string>{U"kansˈaðo o kansˈaða"});
    CHECK(phonemizeSentences(" ... ").empty());
}
