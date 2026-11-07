#pragma once

#include <string>
#include <string_view>
#include <vector>

// What the voice reads, and how. speechText() turns the course's symbols into words, for any voice.
// The phonemes feed the bundled neural voice: they follow espeak-ng's IPA for Latin-American Spanish
// (es-419), which the voice was trained on, without linking espeak-ng (GPL-3.0). tests/speech_tests.cpp
// checks them against espeak-ng's own output (tests/data/phonemes_es419.json).
namespace lq {

// The bundled voice's narrator: everything the app reads outside a dialogue, unless the learner picks another
// voice in Settings.
inline constexpr std::string_view kNarratorVoice = "M2";

// A bundled voice the learner can pick in Settings for everything outside dialogues, whose cast stays fixed.
struct VoiceChoice {
    std::string_view style;   // the style file's name, "F3"
    std::string_view name;    // what Settings calls it
    std::string_view detail;  // how it sounds, in a few words
};
// The voices on offer, the clearest first; kNarratorVoice is the first.
const std::vector<VoiceChoice> &voiceChoices();

// How fast the bundled voices speak, as how many times longer than kReferenceSyllablesPerSecond (above 1 is
// slower; Settings shows its inverse, the speed: ×1.2 is 0.83×). Every voice is brought to the same pace:
// their own paces run from 4.0 (M5) to 5.2 (F4) syllables a second, so one stretch for all left the slow
// voices dragging. The recommended paces follow the usual advice for beginners, 0.75× to 0.85× of a voice's
// normal speed. Settings moves both in steps of 0.05 within these ranges.
inline constexpr double kReferenceSyllablesPerSecond = 4.8;  // 1×: the voices' own average pace, M2's
inline constexpr double kDefaultPace = 1.2;                  // 0.83×, 4.0 syllables a second
inline constexpr double kMinPace = 1.0;
inline constexpr double kMaxPace = 1.4;
inline constexpr double kDefaultSlowPace = 1.35;  // 0.74×, 3.6 syllables a second
inline constexpr double kMinSlowPace = 1.15;
inline constexpr double kMaxSlowPace = 1.8;
inline constexpr double kSlowPaceGap = 0.15;  // "slow" is always at least this much slower than normal
// Seconds of silence between two sentences: at normal pace as advised for intermediate learners (0.5 to
// 0.8 s), at slow pace as for beginners.
inline constexpr double kSentencePause = 0.6;
inline constexpr double kSlowSentencePause = 1.5;

// A bundled voice's own pace in syllables a second, as measured on 36 course lines; 0 for one it does not have.
double voiceSyllablesPerSecond(std::string_view style);
// How much longer than its own pace `style` must speak to reach `pace`: below 1 when a slow voice must speak
// quicker than it does. 1 for a voice it does not know.
double voiceStretch(std::string_view style, double pace);

// The bundled voice a dialogue speaker talks with: women with female voices, men with male ones, and the
// learner ("Tú", whom the course writes as a man: "estoy nervioso") with M2, the narrator's. No character
// shares the learner's voice. Empty for a speaker the course does not have; the narrator speaks those.
std::string_view voiceFor(std::string_view speaker);

// Symbols as a teacher would say them: "cansado / cansada" → "cansado o cansada", "él/ella/usted" →
// "él, ella o usted", "siete/ocho" → "siete u ocho", "tener que + infinitivo" → "tener que más infinitivo",
// "cerca (de)" → "cerca de", "más … que" → "más, que". Numbers become words ("305" → "trescientos cinco",
// "4:30" → "cuatro y media", "B1" → "B uno"), and names a Spanish voice misreads are spelt as they are
// said ("Ohio" → "Ojáio"). UTF-8 in and out.
std::string speechText(std::string_view text);

// The phonemes of one clause without punctuation, words separated by single spaces (UTF-8).
std::string phonemizeClause(std::string_view clause);

// Text already passed through speechText(), as the voice speaks it: one entry per sentence, phonemes as code
// points, with the punctuation Piper voices were trained with ("," ";" ":" followed by a space inside a
// sentence, "." "?" "!" at its end).
std::vector<std::u32string> phonemizeSentences(std::string_view text);

} // namespace lq
