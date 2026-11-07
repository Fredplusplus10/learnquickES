#include "lq/speech.h"

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <utility>

// The phonemizer is a port of desktop/tools/phonemizer_prototype.py; keep the two in step.
namespace lq {
namespace {

// ---- UTF-8 and letters ---------------------------------------------------------------------------------

std::u32string decode(std::string_view text)
{
    std::u32string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size();) {
        const auto lead = static_cast<unsigned char>(text[i]);
        int length = 0;
        char32_t cp = 0;
        if (lead < 0x80) {
            length = 1;
            cp = lead;
        } else if ((lead >> 5) == 0x6) {
            length = 2;
            cp = lead & 0x1Fu;
        } else if ((lead >> 4) == 0xE) {
            length = 3;
            cp = lead & 0x0Fu;
        } else if ((lead >> 3) == 0x1E) {
            length = 4;
            cp = lead & 0x07u;
        }
        bool valid = length > 0 && i + static_cast<std::size_t>(length) <= text.size();
        for (int k = 1; valid && k < length; ++k) {
            const auto next = static_cast<unsigned char>(text[i + static_cast<std::size_t>(k)]);
            valid = (next >> 6) == 0x2;
            cp = (cp << 6) | (next & 0x3Fu);
        }
        if (!valid) {
            out.push_back(U'�');
            ++i;
            continue;
        }
        out.push_back(cp);
        i += static_cast<std::size_t>(length);
    }
    return out;
}

std::string encode(std::u32string_view text)
{
    std::string out;
    out.reserve(text.size());
    for (const char32_t cp : text) {
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else if (cp < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else if (cp < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
            out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
        }
    }
    return out;
}

char32_t lower(char32_t c)
{
    const bool ascii = c >= U'A' && c <= U'Z';
    const bool latin1 = c >= 0xC0 && c <= 0xDE && c != 0xD7;  // À-Þ, the Latin-1 capitals
    return ascii || latin1 ? static_cast<char32_t>(c + 32) : c;
}

// Lowercase, with a decomposed accent, tilde or diaeresis joined to its letter ("e" + U+0301 → "é").
std::u32string prepare(std::u32string_view text)
{
    static const std::map<std::pair<char32_t, char32_t>, char32_t> kCompose{
        {{U'a', 0x301}, U'á'}, {{U'e', 0x301}, U'é'}, {{U'i', 0x301}, U'í'}, {{U'o', 0x301}, U'ó'},
        {{U'u', 0x301}, U'ú'}, {{U'n', 0x303}, U'ñ'}, {{U'u', 0x308}, U'ü'},
    };
    std::u32string out;
    for (const char32_t c : text) {
        const char32_t low = lower(c);
        if (!out.empty()) {
            const auto it = kCompose.find({out.back(), low});
            if (it != kCompose.end()) {
                out.back() = it->second;
                continue;
            }
        }
        out.push_back(low);
    }
    return out;
}

bool isDigit(char32_t c)
{
    return c >= U'0' && c <= U'9';
}

bool isLetter(char32_t c)
{
    return (c >= U'a' && c <= U'z') || (c >= 0xDF && c <= 0xFF && c != 0xF7);
}

bool isSpace(char32_t c)
{
    return c == U' ' || c == U'\t' || c == U'\n' || c == U'\r' || c == U'\f' || c == U'\v' || c == 0xA0;
}

bool isPlainVowel(char32_t c)
{
    return c == U'a' || c == U'e' || c == U'i' || c == U'o' || c == U'u';
}

// á → a…; 0 for anything else.
char32_t accentBase(char32_t c)
{
    switch (c) {
    case U'á': return U'a';
    case U'é': return U'e';
    case U'í': return U'i';
    case U'ó': return U'o';
    case U'ú': return U'u';
    default: return 0;
    }
}

bool isVowelLetter(char32_t c)
{
    return isPlainVowel(c) || accentBase(c) != 0;
}

bool isFrontVowel(char32_t c)
{
    return c == U'e' || c == U'i' || c == U'é' || c == U'í';
}

constexpr char32_t kPrimary = U'ˈ';
constexpr char32_t kSecondary = U'ˌ';

// ---- Words -----------------------------------------------------------------------------------------------

// Unstressed unless they end the clause; reduced to a secondary stress unless they end it.
const std::array<std::u32string_view, 33> kUnstressed{
    U"a",  U"al", U"con", U"de",  U"del", U"el",  U"en",  U"la", U"las", U"le", U"les",
    U"lo", U"los", U"me", U"mi",  U"mis", U"nos", U"os",  U"por", U"que", U"se", U"si",
    U"sin", U"su", U"sus", U"te", U"tu",  U"tus", U"y",   U"e",  U"o",   U"u",  U"ni",
};
const std::array<std::u32string_view, 9> kSecondaryOnly{
    U"aunque", U"cuando", U"desde", U"para", U"porque", U"como", U"donde", U"pero", U"mientras",
};

template <std::size_t N>
bool contains(const std::array<std::u32string_view, N> &list, std::u32string_view word)
{
    for (const std::u32string_view w : list) {
        if (w == word)
            return true;
    }
    return false;
}

// Whole words espeak-ng pronounces against the rules.
const std::map<std::u32string, std::u32string> &exceptions()
{
    static const std::map<std::u32string, std::u32string> kExceptions{
        {U"méxico", U"mˈɛxiko"},
        {U"mexicano", U"mˌexikˈano"},
        {U"mexicana", U"mˌexikˈana"},
        {U"ser", U"sˈer"},
    };
    return kExceptions;
}

// Letter names, for words with no vowel ("B1", "vs").
std::u32string_view letterName(char32_t c)
{
    switch (c) {
    case U'b': return U"be";
    case U'c': return U"ce";
    case U'd': return U"de";
    case U'f': return U"efe";
    case U'g': return U"ge";
    case U'h': return U"hache";
    case U'j': return U"jota";
    case U'k': return U"ka";
    case U'l': return U"ele";
    case U'm': return U"eme";
    case U'n': return U"ene";
    case U'ñ': return U"eñe";
    case U'p': return U"pe";
    case U'q': return U"cu";
    case U'r': return U"erre";
    case U's': return U"ese";
    case U't': return U"te";
    case U'v': return U"be";
    case U'w': return U"doble be";
    case U'x': return U"equis";
    case U'z': return U"zeta";
    default: return {};
    }
}

void appendWords(std::vector<std::u32string> &out, std::u32string_view text)
{
    std::u32string word;
    for (const char32_t c : text) {
        if (c == U' ') {
            if (!word.empty())
                out.push_back(std::move(word));
            word.clear();
        } else {
            word.push_back(c);
        }
    }
    if (!word.empty())
        out.push_back(std::move(word));
}

void numberWords(int n, std::vector<std::u32string> &out)
{
    static const std::array<std::u32string_view, 30> kUnits{
        U"",          U"uno",          U"dos",         U"tres",        U"cuatro",     U"cinco",
        U"seis",      U"siete",        U"ocho",        U"nueve",       U"diez",       U"once",
        U"doce",      U"trece",        U"catorce",     U"quince",      U"dieciséis",  U"diecisiete",
        U"dieciocho", U"diecinueve",   U"veinte",      U"veintiuno",   U"veintidós",  U"veintitrés",
        U"veinticuatro", U"veinticinco", U"veintiséis", U"veintisiete", U"veintiocho", U"veintinueve",
    };
    static const std::array<std::u32string_view, 10> kTens{
        U"", U"", U"", U"treinta", U"cuarenta", U"cincuenta", U"sesenta", U"setenta", U"ochenta", U"noventa",
    };
    static const std::array<std::u32string_view, 10> kHundreds{
        U"",           U"ciento",      U"doscientos",  U"trescientos", U"cuatrocientos",
        U"quinientos", U"seiscientos", U"setecientos", U"ochocientos", U"novecientos",
    };
    if (n == 0) {
        out.emplace_back(U"cero");
        return;
    }
    if (n >= 1000) {
        const int thousands = n / 1000;
        if (thousands > 1)
            numberWords(thousands, out);
        out.emplace_back(U"mil");
        n %= 1000;
    }
    if (n >= 100) {
        out.emplace_back(n == 100 ? U"cien" : kHundreds[static_cast<std::size_t>(n / 100)]);
        n %= 100;
    }
    if (n >= 30) {
        out.emplace_back(kTens[static_cast<std::size_t>(n / 10)]);
        if (n % 10 != 0) {
            out.emplace_back(U"y");
            out.emplace_back(kUnits[static_cast<std::size_t>(n % 10)]);
        }
    } else if (n > 0) {
        out.emplace_back(kUnits[static_cast<std::size_t>(n)]);
    }
}

// Lowercase words of a clause: letters and digits split apart, numbers spelled out.
std::vector<std::u32string> tokens(std::u32string_view clause)
{
    const std::u32string text = prepare(clause);
    std::vector<std::u32string> out;
    std::u32string run;
    bool digits = false;
    auto flush = [&] {
        if (run.empty())
            return;
        if (!digits) {
            out.push_back(run);
        } else if (run.size() <= 6) {
            numberWords(std::stoi(encode(run)), out);
        } else {
            for (const char32_t d : run)  // a long number is read digit by digit
                numberWords(static_cast<int>(d - U'0'), out);
        }
        run.clear();
    };
    for (const char32_t c : text) {
        if (isDigit(c) || isLetter(c)) {
            if (!run.empty() && isDigit(c) != digits)
                flush();
            digits = isDigit(c);
            run.push_back(c);
        } else {
            flush();
        }
    }
    flush();
    return out;
}

// ---- One word ------------------------------------------------------------------------------------------

// A sound before the clause's context rules. B, D and G stand for b/β, d/ð and ɡ/ɣ until then.
struct Seg {
    std::u32string ph;
    char kind = 'C';  // 'V' vowel, 'C' consonant, 'H' a silent h that keeps vowels apart
    bool accent = false;
    bool firm = false;   // ü: never joins the next vowel
    bool keep = false;   // a weak vowel that stays a vowel inside its diphthong ("cliente", "ruido")
    bool glide = false;
    int syl = -1;
};

Seg consonant(std::u32string_view ph)
{
    Seg seg;
    seg.ph = ph;
    return seg;
}

Seg vowel(char32_t ph)
{
    Seg seg;
    seg.ph = std::u32string(1, ph);
    seg.kind = 'V';
    return seg;
}

std::vector<Seg> segments(const std::u32string &w)
{
    std::vector<Seg> out;
    const std::size_t n = w.size();
    auto at = [&](std::size_t j) -> char32_t { return j < n ? w[j] : 0; };
    auto lastIs = [&](std::u32string_view ph) { return !out.empty() && out.back().ph == ph; };
    for (std::size_t i = 0; i < n; ++i) {
        const char32_t c = w[i];
        const char32_t next = at(i + 1);
        if (isVowelLetter(c)) {
            // After a trilled r, or an l / r that starts the word or follows a consonant, a weak vowel keeps
            // its vowel sound: "cliente", "luego", "ruido", "barrio" (but "familia", "historia").
            bool keep = false;
            if ((c == U'i' || c == U'u') && !out.empty()) {
                const Seg &liquid = out.back();
                if (liquid.ph == U"l" || liquid.ph == U"r" || liquid.ph == U"ɾ") {
                    const Seg *before = out.size() > 1 ? &out[out.size() - 2] : nullptr;
                    keep = liquid.ph == U"r" || before == nullptr || before->kind == 'C';
                }
            }
            Seg seg = vowel(accentBase(c) != 0 ? accentBase(c) : c);
            seg.accent = accentBase(c) != 0;
            seg.keep = keep;
            out.push_back(seg);
        } else if (c == U'ü') {
            Seg seg = vowel(U'u');
            seg.firm = true;
            out.push_back(seg);
        } else if (c == U'y') {
            // A consonant before a vowel ("yo", "ayer"), a vowel otherwise ("hoy", "y"); "muy" ends in j.
            if (next != 0 && isVowelLetter(next))
                out.push_back(consonant(U"ʝ"));
            else if (!out.empty() && out.back().kind == 'V' && out.back().ph == U"u" && next == 0)
                out.push_back(consonant(U"j"));
            else
                out.push_back(vowel(U'i'));
        } else if (c == U'b' || c == U'v') {
            out.push_back(consonant(U"B"));
        } else if (c == U'd') {
            out.push_back(consonant(U"D"));
        } else if (c == U'g') {
            if (isFrontVowel(next)) {
                out.push_back(consonant(U"x"));
            } else {
                out.push_back(consonant(U"G"));
                if (next == U'u' && isFrontVowel(at(i + 2)))
                    ++i;  // "guerra": the u is silent
            }
        } else if (c == U'j') {
            out.push_back(consonant(U"x"));
        } else if (c == U'c') {
            if (next == U'h') {
                out.push_back(consonant(U"tʃ"));
                ++i;
            } else if (isFrontVowel(next)) {
                if (!lastIs(U"s"))  // "ascenso"
                    out.push_back(consonant(U"s"));
            } else {
                out.push_back(consonant(U"k"));
            }
        } else if (c == U'q') {
            out.push_back(consonant(U"k"));
            if (next == U'u')
                ++i;
        } else if (c == U'z') {
            out.push_back(consonant(U"s"));
        } else if (c == U'l') {
            if (next == U'l') {
                // Between vowels espeak-ng doubles it ("calle" → kˈajje); at the start it is ʝ ("llave").
                if (!out.empty()) {
                    out.push_back(consonant(U"j"));
                    out.push_back(consonant(U"j"));
                } else {
                    out.push_back(consonant(U"ʝ"));
                }
                ++i;
            } else {
                out.push_back(consonant(U"l"));
            }
        } else if (c == U'r') {
            if (next == U'r') {
                out.push_back(consonant(U"r"));
                ++i;
            } else if (i == 0 || lastIs(U"n") || lastIs(U"l") || lastIs(U"s")) {
                out.push_back(consonant(U"r"));
            } else {
                out.push_back(consonant(U"ɾ"));
            }
        } else if (c == U'ñ') {
            out.push_back(consonant(U"ɲ"));
        } else if (c == U'h') {
            Seg seg;
            seg.kind = 'H';  // silent, but it keeps vowels apart: "prohibir", "Ohio"
            out.push_back(seg);
        } else if (c == U'x') {
            if (i > 0)  // "xilófono" starts with s
                out.push_back(consonant(U"k"));
            out.push_back(consonant(U"s"));
        } else if (c == U'k') {
            out.push_back(consonant(U"k"));
        } else if (c == U'w') {
            out.push_back(vowel(U'u'));
        } else if (c == U'p' && i == 0 && next == U's') {
            // "psicología": the p is silent
        } else if (c == U'f' || c == U'm' || c == U'n' || c == U'p' || c == U't') {
            out.push_back(consonant(std::u32string(1, c)));
        } else if (c == U's') {
            if (!lastIs(U"s"))
                out.push_back(consonant(U"s"));
        }
    }
    return out;
}

bool weak(const Seg &s)
{
    return s.kind == 'V' && (s.ph == U"i" || s.ph == U"u") && !s.accent && !s.firm;
}

bool clusterFirst(const std::u32string &ph)
{
    return ph == U"p" || ph == U"B" || ph == U"f" || ph == U"k" || ph == U"G" || ph == U"t" || ph == U"D";
}

bool clusterSecond(const std::u32string &ph)
{
    return ph == U"l" || ph == U"ɾ";
}

// Gives every segment its syllable and returns the nucleus segment of each syllable.
std::vector<std::size_t> syllabify(std::vector<Seg> &segs)
{
    std::vector<std::vector<std::size_t>> groups;  // vowels that share a syllable
    for (std::size_t i = 0; i < segs.size();) {
        if (segs[i].kind != 'V') {
            ++i;
            continue;
        }
        std::size_t j = i + 1;
        while (j < segs.size() && segs[j].kind == 'V')
            ++j;
        // Two strong vowels, an accented weak vowel or a ü break the run into syllables.
        std::vector<std::size_t> current{i};
        for (std::size_t k = i + 1; k < j; ++k) {
            const Seg &prev = segs[current.back()];
            const Seg &s = segs[k];
            bool allWeak = true;
            for (const std::size_t x : current)
                allWeak = allWeak && weak(segs[x]);
            const bool together = (weak(prev) || weak(s)) && (weak(s) || allWeak) && !prev.firm && !s.firm;
            if (together && current.size() < 3) {
                current.push_back(k);
            } else {
                groups.push_back(current);
                current = {k};
            }
        }
        groups.push_back(current);
        i = j;
    }

    std::vector<std::size_t> nuclei;
    for (const auto &group : groups) {
        // The strong or accented vowel; with two weak vowels, the second.
        std::size_t nucleus = group.back();
        for (const std::size_t k : group) {
            if (!weak(segs[k])) {
                nucleus = k;
                break;
            }
        }
        nuclei.push_back(nucleus);
        for (const std::size_t k : group) {
            segs[k].syl = static_cast<int>(nuclei.size()) - 1;
            segs[k].glide = k != nucleus;
        }
    }

    // Consonants between two vowel groups: one goes to the next syllable, and so do p/b/f/k/g/t/d + l/r.
    for (std::size_t g = 0; g < groups.size(); ++g) {
        const std::size_t start = groups[g].back() + 1;
        const bool last = g + 1 == groups.size();
        const std::size_t end = last ? segs.size() : groups[g + 1].front();
        const std::size_t count = end - start;
        std::size_t split = count;  // how many stay in the current syllable
        if (!last) {
            if (count == 1) {
                split = 0;
            } else if (count >= 2) {
                const std::u32string &a = segs[end - 2].ph;
                const std::u32string &b = segs[end - 1].ph;
                const bool cluster = clusterFirst(a) && clusterSecond(b) && !(a == U"D" && b == U"l");
                split = count - (cluster ? 2 : 1);
            }
        }
        for (std::size_t k = 0; k < count; ++k)
            segs[start + k].syl = static_cast<int>(k < split ? g : g + 1);
    }
    const std::size_t firstVowel = groups.empty() ? segs.size() : groups.front().front();
    for (std::size_t k = 0; k < firstVowel; ++k)
        segs[k].syl = 0;
    return nuclei;
}

int primaryStress(const std::vector<Seg> &segs, std::size_t syllables, const std::u32string &word)
{
    for (const Seg &s : segs) {
        if (s.accent)
            return s.syl;
    }
    if (syllables <= 1)
        return 0;
    const char32_t last = word.back();
    const bool penultimate = isPlainVowel(last) || last == U'n' || last == U's';
    return static_cast<int>(syllables) - (penultimate ? 2 : 1);
}

enum class Stress { Primary, Secondary, None };

std::vector<std::u32string> wordPhonemes(const std::u32string &word, Stress stress)
{
    constexpr std::u32string_view kMente = U"mente";
    if (stress == Stress::Primary && word.size() > 7 && word.ends_with(kMente)) {
        // "rápidamente": the adjective keeps its stress and "mente" has its own.
        std::vector<std::u32string> out = wordPhonemes(word.substr(0, word.size() - kMente.size()), stress);
        const std::vector<std::u32string> suffix = wordPhonemes(std::u32string(kMente), stress);
        out.insert(out.end(), suffix.begin(), suffix.end());
        return out;
    }
    std::vector<Seg> segs = segments(word);
    std::vector<std::u32string> out;
    if (segs.empty())
        return out;
    const std::vector<std::size_t> nuclei = syllabify(segs);
    if (nuclei.empty()) {
        for (const Seg &s : segs)
            out.push_back(s.ph);
        return out;
    }
    const int primary = primaryStress(segs, nuclei.size(), word);
    std::map<int, char32_t> marks;
    if (stress == Stress::Primary) {
        marks[primary] = kPrimary;
        // Secondary stress on every other syllable from the start, at least two before the primary one.
        for (int k = 0; k < primary - 1; k += 2)
            marks[k] = kSecondary;
    } else if (stress == Stress::Secondary) {
        marks[primary] = kSecondary;
    }

    for (std::size_t k = 0; k < segs.size(); ++k) {
        const Seg &s = segs[k];
        const auto mark = s.syl >= 0 ? marks.find(s.syl) : marks.end();
        const bool isNucleus = s.kind == 'V' && s.syl >= 0 && nuclei[static_cast<std::size_t>(s.syl)] == k;
        if (isNucleus && mark != marks.end())
            out.emplace_back(1, mark->second);
        std::u32string ph = s.ph;
        if (s.kind == 'V' && s.glide && !s.keep) {
            if (k < nuclei[static_cast<std::size_t>(s.syl)]) {
                ph = ph == U"i" ? U"j" : U"w";
            } else {
                ph = ph == U"i" ? U"ɪ" : U"ʊ";
                if (ph == U"ʊ" && k > 0 && segs[k - 1].ph == U"o")
                    ph = U"w";  // "estadounidense"
            }
        }
        // A stressed e before n + consonant is open: "presente" → pɾesˈɛnte, "tengo" → tˈɛŋɡo.
        if (ph == U"e" && s.kind == 'V' && !s.glide && mark != marks.end() && mark->second == kPrimary
            && word != kMente && k + 2 < segs.size()) {
            const Seg &n = segs[k + 1];
            const Seg &after = segs[k + 2];
            if (n.ph == U"n" && after.kind == 'C' && after.syl != s.syl)
                ph = U"ɛ";
        }
        out.push_back(ph);
    }
    return out;
}

// ---- The clause ------------------------------------------------------------------------------------------

bool isMark(const std::u32string &ph)
{
    return ph.size() == 1 && (ph[0] == kPrimary || ph[0] == kSecondary);
}

// Context rules across the words of a clause: b/d/g as stops or fricatives, n before labials and velars.
std::u32string realize(const std::vector<std::vector<std::u32string>> &words)
{
    struct Entry {
        std::u32string ph;
        std::size_t word;
    };
    std::vector<Entry> flat;
    for (std::size_t w = 0; w < words.size(); ++w) {
        for (const std::u32string &ph : words[w]) {
            if (!ph.empty())  // the silent h has done its job
                flat.push_back({ph, w});
        }
    }
    std::vector<std::u32string> out(words.size());
    std::u32string prev;  // the previous sound as realized; empty at the start of the clause
    for (std::size_t i = 0; i < flat.size(); ++i) {
        const Entry &e = flat[i];
        if (isMark(e.ph)) {
            out[e.word] += e.ph;
            continue;
        }
        const Entry *following = nullptr;
        for (std::size_t j = i + 1; j < flat.size(); ++j) {
            if (!isMark(flat[j].ph)) {
                following = &flat[j];
                break;
            }
        }
        const std::u32string next = following ? following->ph : std::u32string();
        const bool sameWord = following != nullptr && following->word == e.word;
        const bool wordFinal = !sameWord;
        std::u32string ph = e.ph;
        if (ph == U"B") {
            ph = prev.empty() || prev == U"m" || prev == U"n" || wordFinal ? U"b" : U"β";
        } else if (ph == U"D") {
            ph = prev.empty() || prev == U"n" || prev == U"m" || wordFinal ? U"d" : U"ð";
        } else if (ph == U"G") {
            ph = prev.empty() || prev == U"n" || prev == U"ŋ" || next == U"n" || wordFinal ? U"ɡ" : U"ɣ";
        } else if (ph == U"n") {
            if (next == U"B" || next == U"p" || (next == U"f" && !sameWord))
                ph = U"m";
            else if ((next == U"G" || next == U"x") && sameWord)
                ph = U"ŋ";
        }
        out[e.word] += ph;
        prev = ph;
    }
    std::u32string result;
    for (const std::u32string &word : out) {
        if (word.empty())
            continue;
        if (!result.empty())
            result += U' ';
        result += word;
    }
    return result;
}

std::u32string phonemize(std::u32string_view clause)
{
    std::vector<std::u32string> words;
    for (std::u32string &token : tokens(clause)) {
        bool hasVowel = false;
        for (const char32_t c : token)
            hasVowel = hasVowel || isVowelLetter(c) || c == U'y';
        if (hasVowel) {
            words.push_back(std::move(token));
        } else {
            for (const char32_t c : token)
                appendWords(words, letterName(c));
        }
    }
    std::vector<std::vector<std::u32string>> phonemes;
    for (std::size_t i = 0; i < words.size(); ++i) {
        const std::u32string &w = words[i];
        const auto exception = exceptions().find(w);
        if (exception != exceptions().end()) {
            std::vector<std::u32string> chars;
            for (const char32_t c : exception->second)
                chars.emplace_back(1, c);
            phonemes.push_back(std::move(chars));
            continue;
        }
        const bool last = i + 1 == words.size();
        Stress stress = Stress::Primary;
        if (!last && contains(kUnstressed, w))
            stress = Stress::None;
        else if (!last && contains(kSecondaryOnly, w))
            stress = Stress::Secondary;
        phonemes.push_back(wordPhonemes(w, stress));
    }
    return realize(phonemes);
}

// ---- Symbols -------------------------------------------------------------------------------------------

std::u32string trim(std::u32string_view text)
{
    std::size_t start = 0;
    std::size_t end = text.size();
    while (start < end && isSpace(text[start]))
        ++start;
    while (end > start && isSpace(text[end - 1]))
        --end;
    return std::u32string(text.substr(start, end - start));
}

// "o", or "u" before a word that starts with the sound o ("siete u ocho", "uno u hoja").
std::u32string orFor(std::u32string_view nextPart)
{
    const std::u32string next = prepare(trim(nextPart));
    const bool oSound = !next.empty()
                        && (next[0] == U'o' || next[0] == U'ó'
                            || (next[0] == U'h' && next.size() > 1 && (next[1] == U'o' || next[1] == U'ó')));
    return oSound ? U" u " : U" o ";
}

// ---- Numbers and names -----------------------------------------------------------------------------------

// Spanish cardinals: 305 → "trescientos cinco", 2019 → "dos mil diecinueve". Neural voices guess at digits
// ("305" came out "300-105"), so every number is written out before it reaches one.
std::u32string cardinal(unsigned long n)
{
    static const std::u32string_view small[] = {
        U"cero", U"uno", U"dos", U"tres", U"cuatro", U"cinco", U"seis", U"siete", U"ocho", U"nueve", U"diez",
        U"once", U"doce", U"trece", U"catorce", U"quince", U"dieciséis", U"diecisiete", U"dieciocho",
        U"diecinueve", U"veinte", U"veintiuno", U"veintidós", U"veintitrés", U"veinticuatro", U"veinticinco",
        U"veintiséis", U"veintisiete", U"veintiocho", U"veintinueve"};
    static const std::u32string_view tens[] = {U"", U"", U"", U"treinta", U"cuarenta", U"cincuenta", U"sesenta",
                                               U"setenta", U"ochenta", U"noventa"};
    static const std::u32string_view hundreds[] = {U"", U"ciento", U"doscientos", U"trescientos",
                                                   U"cuatrocientos", U"quinientos", U"seiscientos",
                                                   U"setecientos", U"ochocientos", U"novecientos"};
    if (n < 30)
        return std::u32string(small[n]);
    if (n < 100)
        return std::u32string(tens[n / 10]) + (n % 10 ? U" y " + std::u32string(small[n % 10]) : U"");
    if (n == 100)
        return U"cien";
    if (n < 1000)
        return std::u32string(hundreds[n / 100]) + (n % 100 ? U" " + cardinal(n % 100) : U"");
    if (n < 1'000'000) {
        const unsigned long thousands = n / 1000;
        std::u32string words = thousands == 1 ? U"mil" : cardinal(thousands) + U" mil";
        if (thousands != 1 && thousands % 10 == 1 && thousands % 100 != 11) {
            // Uno shortens before mil: "treinta y un mil", "veintiún mil".
            words.erase(words.size() - 5, 1);
            if (thousands % 100 == 21)
                words.replace(words.size() - 6, 1, U"ú");
        }
        return words + (n % 1000 ? U" " + cardinal(n % 1000) : U"");
    }
    return U"";  // larger numbers stay as digits
}

// Names whose spelling a Spanish voice reads wrongly, spelt as they are said.
constexpr std::pair<std::u32string_view, std::u32string_view> kPronunciations[] = {
    {U"Ohio", U"Ojáio"},
};

// Numbers as words ("a las 4:30" → "a las cuatro y media", "1.000" → "mil", "B1" → "B uno") and the names
// above spelt as they are said.
std::u32string spellOut(const std::u32string &text)
{
    std::u32string out;
    std::size_t i = 0;
    const auto digitsAt = [&](std::size_t k, std::size_t count) {
        for (std::size_t j = 0; j < count; ++j) {
            if (k + j >= text.size() || !isDigit(text[k + j]))
                return false;
        }
        return k + count >= text.size() || !isDigit(text[k + count]);
    };
    const auto number = [&](std::size_t from, std::size_t to) {
        unsigned long n = 0;
        for (std::size_t k = from; k < to; ++k) {
            if (isDigit(text[k]))
                n = n * 10 + static_cast<unsigned long>(text[k] - U'0');
        }
        return n;
    };
    while (i < text.size()) {
        const char32_t c = text[i];
        const bool wordStart = i == 0 || !(isLetter(lower(text[i - 1])) || isDigit(text[i - 1]));
        if (wordStart && c >= U'A' && c <= U'C' && i + 1 < text.size() && isDigit(text[i + 1])
            && digitsAt(i + 1, 1)) {
            // A level of the European framework: A1, A2, B1. The capital letter alone is read as a letter by
            // every voice; "be uno" lost its "be" with some.
            out += c;
            out += U' ';
            out += cardinal(static_cast<unsigned long>(text[i + 1] - U'0'));
            i += 2;
            continue;
        }
        if (isDigit(c) && wordStart) {
            std::size_t end = i;
            while (end < text.size() && isDigit(text[end]))
                ++end;
            if (end - i <= 2 && end < text.size() && text[end] == U':' && digitsAt(end + 1, 2)) {
                // A time: "4:30" → "cuatro y media".
                const unsigned long minutes = number(end + 1, end + 3);
                out += cardinal(number(i, end));
                if (minutes == 15)
                    out += U" y cuarto";
                else if (minutes == 30)
                    out += U" y media";
                else if (minutes != 0)
                    out += U" y " + cardinal(minutes);
                i = end + 3;
                continue;
            }
            // Thousands written with dots: "1.000", "25.000".
            while (end - i <= 7 && end < text.size() && text[end] == U'.' && digitsAt(end + 1, 3))
                end += 4;
            const std::u32string words = cardinal(number(i, end));
            if (words.empty())
                out.append(text, i, end - i);
            else
                out += words;
            i = end;
            continue;
        }
        bool replaced = false;
        if (wordStart) {
            for (const auto &[written, said] : kPronunciations) {
                const std::size_t end = i + written.size();
                if (text.compare(i, written.size(), written) == 0
                    && (end == text.size() || !isLetter(lower(text[end])))) {
                    out += said;
                    i = end;
                    replaced = true;
                    break;
                }
            }
        }
        if (!replaced)
            out += text[i++];
    }
    return out;
}

} // namespace

std::string speechText(std::string_view text)
{
    const std::u32string in = decode(text);
    std::u32string s;
    for (std::size_t i = 0; i < in.size(); ++i) {
        const char32_t c = in[i];
        if (c == U'…' || c == U'—' || c == U'–' || c == U'→') {
            s += U',';
        } else if (c == U'.' && i + 2 < in.size() && in[i + 1] == U'.' && in[i + 2] == U'.') {
            s += U',';
            i += 2;
        } else if (c == U'_') {
            while (i + 1 < in.size() && in[i + 1] == U'_')
                ++i;
            s += U',';  // a blank to fill is a pause
        } else if (c == U'+') {
            s += U" más ";
        } else if (c == U'(' || c == U')') {
            s += U' ';
        } else if (c == U'/') {
            // Between two numbers ("80/20") it is not read at all.
            std::size_t before = s.size();
            while (before > 0 && isSpace(s[before - 1]))
                --before;
            std::size_t after = i + 1;
            while (after < in.size() && isSpace(in[after]))
                ++after;
            if (before > 0 && isDigit(s[before - 1]) && after < in.size() && isDigit(in[after]))
                s += U' ';
            else
                s += U'/';
        } else {
            s += c;
        }
    }

    // Alternatives: "a / b" → "a o b", "a/b/c" → "a, b o c".
    std::vector<std::u32string> parts;
    std::u32string part;
    for (const char32_t c : s) {
        if (c == U'/') {
            parts.push_back(trim(part));
            part.clear();
        } else {
            part += c;
        }
    }
    parts.push_back(trim(part));
    std::u32string joined = parts[0];
    for (std::size_t k = 1; k < parts.size(); ++k)
        joined += (k + 1 == parts.size() ? orFor(parts[k]) : std::u32string(U", ")) + parts[k];

    // Single spaces, none before a comma, no repeated commas.
    std::u32string out;
    for (const char32_t c : joined) {
        const char32_t ch = isSpace(c) ? U' ' : c;
        if (ch == U' ' && (out.empty() || out.back() == U' '))
            continue;
        if (ch == U',') {
            while (!out.empty() && out.back() == U' ')
                out.pop_back();
            if (!out.empty() && out.back() == U',')
                continue;
            if (out.empty())
                continue;
        }
        out += ch;
    }
    while (!out.empty() && (out.back() == U' ' || out.back() == U','))
        out.pop_back();
    return encode(spellOut(out));
}

const std::vector<VoiceChoice> &voiceChoices()
{
    static const std::vector<VoiceChoice> choices{
        {"M2", "Julián", "Male, the clearest"},
        {"F3", "Clara", "Female, warm and clear"},
        {"M4", "Mateo", "Male, mid pitch"},
        {"F1", "Inés", "Female, calm and unhurried"},
        {"M5", "Tomás", "Male, deeper"},
        {"M1", "Hugo", "Male, brighter"},
    };
    return choices;
}

double voiceSyllablesPerSecond(std::string_view style)
{
    // Syllables over speaking time (pauses left out) on the 36 lines that measure every voice change, each
    // voice at its own pace.
    static constexpr std::pair<std::string_view, double> rates[] = {
        {"F1", 4.72}, {"F2", 4.90}, {"F3", 4.48}, {"F4", 5.20}, {"F5", 4.34},
        {"M1", 4.67}, {"M2", 4.83}, {"M3", 5.15}, {"M4", 4.48}, {"M5", 4.01},
    };
    for (const auto &[name, rate] : rates) {
        if (name == style)
            return rate;
    }
    return 0.0;
}

double voiceStretch(std::string_view style, double pace)
{
    const double rate = voiceSyllablesPerSecond(style);
    return rate > 0 ? pace * rate / kReferenceSyllablesPerSecond : 1.0;
}

std::string_view voiceFor(std::string_view speaker)
{
    // The learner speaks with M2, the clearest voice. Women speak with F3, the clearest female one. Men, in the
    // order the learner meets them, alternate between the next two male voices. On 36 course lines at the
    // app's pace, Whisper missed 1.5 % of the words with M2, 3.0 % with F3 and M4, 4.1 % with M5, 3.8 % with F1
    // and 6.8 % with M1, which left the cast; F4 and F5 missed 11 %, dropping whole words. Every character
    // appears in a unit of their own, so clarity wins over variety.
    static constexpr std::pair<std::string_view, std::string_view> voices[] = {
        {"Tú", "M2"},
        {"Lucía", "F3"}, {"Ana", "F3"}, {"Sofía", "F3"}, {"Jefa", "F3"},
        {"Vendedora", "F3"}, {"Marta", "F3"}, {"Señora", "F3"}, {"Elena", "F3"},
        {"Valeria", "F3"}, {"Amiga", "F3"}, {"Laura", "F3"}, {"Isabel", "F3"},
        {"Profesora", "F3"}, {"Doctora", "F3"}, {"Entrevistadora", "F3"}, {"Recepcionista", "F3"},
        {"Carlos", "M5"}, {"Diego", "M4"}, {"Mesero", "M5"}, {"Pablo", "M4"},
        {"Amigo", "M5"}, {"Andrés", "M4"}, {"Colega", "M5"}, {"Empleado", "M4"},
    };
    for (const auto &[name, voice] : voices) {
        if (name == speaker)
            return voice;
    }
    return {};
}

std::string phonemizeClause(std::string_view clause)
{
    return encode(phonemize(decode(clause)));
}

std::vector<std::u32string> phonemizeSentences(std::string_view text)
{
    std::vector<std::u32string> sentences;
    std::u32string sentence;
    std::u32string clause;
    auto endClause = [&](char32_t punctuation) {
        const std::u32string ph = phonemize(clause);
        clause.clear();
        sentence += ph;
        if (sentence.empty() || punctuation == 0)
            return;
        sentence += punctuation;
        if (punctuation == U',' || punctuation == U';' || punctuation == U':')
            sentence += U' ';
    };
    auto endSentence = [&] {
        if (!sentence.empty())
            sentences.push_back(std::move(sentence));
        sentence.clear();
    };
    for (const char32_t c : decode(text)) {
        if (c == U',' || c == U';' || c == U':') {
            endClause(c);
        } else if (c == U'.' || c == U'?' || c == U'!') {
            endClause(c);
            endSentence();
        } else {
            clause += c;
        }
    }
    endClause(0);
    endSentence();
    return sentences;
}

} // namespace lq
