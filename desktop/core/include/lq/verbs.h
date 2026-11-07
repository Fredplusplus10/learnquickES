#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

// Verb data and conjugator, ported from app/js/core/verbs.js. Regular forms come from rules; irregular verbs
// list only the tenses that deviate. Persons: 0 yo · 1 tú · 2 él/ella/usted · 3 nosotros · 4 vosotros ·
// 5 ellos/ellas/ustedes.
namespace lq {

enum class Tense { Present, NearFuture, Preterite, Imperfect, Perfect, Future, Conditional, Subjunctive };

inline constexpr std::array<Tense, 8> kTenses{Tense::Present,  Tense::NearFuture, Tense::Preterite,
                                              Tense::Imperfect, Tense::Perfect,    Tense::Future,
                                              Tense::Conditional, Tense::Subjunctive};

struct TenseInfo {
    std::string_view id;  // as in the web app: "pres", "irA", "pret"…
    std::string_view en;
    std::string_view es;
    std::string_view cue;
    int unit;  // the lesson (0-based) that introduces it; 0 for the present
};

const TenseInfo &tenseInfo(Tense tense);
std::optional<Tense> tenseFromId(std::string_view id);

struct Verb {
    std::string_view infinitive;
    std::string_view en;
    bool core;  // the eight 80/20 verbs
};

const std::vector<Verb> &verbs();
const Verb *findVerb(std::string_view infinitive);
std::vector<std::string_view> coreVerbs();

using Paradigm = std::array<std::string, 6>;
Paradigm conjugate(std::string_view infinitive, Tense tense);
std::string participle(std::string_view infinitive);
// Subject pronouns for each person, e.g. {"él", "ella", "usted"}.
const std::vector<std::string_view> &pronouns(int person);

} // namespace lq
