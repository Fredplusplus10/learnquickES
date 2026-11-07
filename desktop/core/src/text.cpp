#include "lq/text.h"

#include <algorithm>

namespace lq {

namespace {

// Decodes one UTF-8 sequence at text[i] and advances i. Malformed bytes become U+FFFD.
char32_t decode(std::string_view text, std::size_t &i)
{
    const auto byte = [&](std::size_t k) { return static_cast<unsigned char>(text[k]); };
    const unsigned char lead = byte(i);
    int extra = 0;
    char32_t cp = 0;
    if (lead < 0x80) {
        ++i;
        return lead;
    }
    if ((lead & 0xE0) == 0xC0) {
        extra = 1;
        cp = lead & 0x1F;
    } else if ((lead & 0xF0) == 0xE0) {
        extra = 2;
        cp = lead & 0x0F;
    } else if ((lead & 0xF8) == 0xF0) {
        extra = 3;
        cp = lead & 0x07;
    } else {
        ++i;
        return 0xFFFD;
    }
    if (i + static_cast<std::size_t>(extra) >= text.size()) {
        i = text.size();
        return 0xFFFD;
    }
    for (int k = 1; k <= extra; ++k) {
        const unsigned char next = byte(i + static_cast<std::size_t>(k));
        if ((next & 0xC0) != 0x80) {
            ++i;
            return 0xFFFD;
        }
        cp = (cp << 6) | (next & 0x3F);
    }
    i += static_cast<std::size_t>(extra) + 1;
    return cp;
}

void encode(char32_t cp, std::string &out)
{
    if (cp < 0x80) {
        out += static_cast<char>(cp);
    } else if (cp < 0x800) {
        out += static_cast<char>(0xC0 | (cp >> 6));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else if (cp < 0x10000) {
        out += static_cast<char>(0xE0 | (cp >> 12));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (cp >> 18));
        out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (cp & 0x3F));
    }
}

template <typename F>
std::string mapCodePoints(std::string_view text, F &&map)
{
    std::string out;
    out.reserve(text.size());
    for (std::size_t i = 0; i < text.size();) {
        const char32_t cp = map(decode(text, i));
        if (cp != 0)
            encode(cp, out);
    }
    return out;
}

char32_t lower(char32_t cp)
{
    if (cp >= U'A' && cp <= U'Z')
        return cp + 32;
    // À–Þ, except the multiplication sign.
    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7)
        return cp + 32;
    return cp;
}

// The same characters as \s in JavaScript.
bool isSpace(char32_t cp)
{
    switch (cp) {
    case U' ': case U'\t': case U'\n': case U'\v': case U'\f': case U'\r':
    case 0xA0: case 0x1680: case 0x2028: case 0x2029: case 0x202F: case 0x205F: case 0x3000: case 0xFEFF:
        return true;
    default:
        return cp >= 0x2000 && cp <= 0x200A;
    }
}

// /[¿?¡!.,;:"“”«»()]/ in util.js.
bool isPunctuation(char32_t cp)
{
    switch (cp) {
    case U'¿': case U'?': case U'¡': case U'!': case U'.': case U',': case U';': case U':':
    case U'"': case U'“': case U'”': case U'«': case U'»': case U'(': case U')':
        return true;
    default:
        return false;
    }
}

// Base letter of a precomposed Latin-1 letter (its canonical decomposition minus the marks).
char32_t baseLetter(char32_t cp)
{
    // U+00C0–U+00FF. Zero where the letter has no decomposition: Æ, Ð, ×, Ø, Þ, ß, ÷ and their kin.
    static constexpr char32_t table[64] = {
        'A', 'A', 'A', 'A', 'A', 'A', 0, 'C', 'E', 'E', 'E', 'E', 'I', 'I', 'I', 'I',
        0,   'N', 'O', 'O', 'O', 'O', 'O', 0, 0, 'U', 'U', 'U', 'U', 'Y', 0, 0,
        'a', 'a', 'a', 'a', 'a', 'a', 0, 'c', 'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
        0,   'n', 'o', 'o', 'o', 'o', 'o', 0, 0, 'u', 'u', 'u', 'u', 'y', 0, 'y',
    };
    if (cp >= 0xC0 && cp <= 0xFF && table[cp - 0xC0] != 0)
        return table[cp - 0xC0];
    return cp;
}

} // namespace

std::string toLower(std::string_view text)
{
    return mapCodePoints(text, lower);
}

std::string normalize(std::string_view text)
{
    const std::string spaced = mapCodePoints(text, [](char32_t cp) {
        cp = lower(cp);
        return isPunctuation(cp) || isSpace(cp) ? U' ' : cp;
    });
    std::string out;
    out.reserve(spaced.size());
    for (const char c : spaced) {
        if (c == ' ' && (out.empty() || out.back() == ' '))
            continue;
        out += c;
    }
    if (!out.empty() && out.back() == ' ')
        out.pop_back();
    return out;
}

std::string stripAccents(std::string_view text)
{
    return mapCodePoints(text, [](char32_t cp) -> char32_t {
        // Combining diacritical marks, for text that arrives already decomposed.
        if (cp >= 0x300 && cp <= 0x36F)
            return 0;
        return baseLetter(cp);
    });
}

std::vector<std::string> words(std::string_view text)
{
    std::vector<std::string> out;
    const std::string normalized = normalize(text);
    std::size_t start = 0;
    while (start < normalized.size()) {
        const std::size_t end = std::min(normalized.find(' ', start), normalized.size());
        out.emplace_back(normalized.substr(start, end - start));
        start = end + 1;
    }
    return out;
}

std::size_t codePointCount(std::string_view text)
{
    return static_cast<std::size_t>(std::count_if(text.begin(), text.end(), [](char c) {
        return (static_cast<unsigned char>(c) & 0xC0) != 0x80;
    }));
}

AnswerCheck checkAnswer(std::string_view input, const std::vector<std::string> &answers)
{
    AnswerCheck result;
    result.expected = answers.empty() ? std::string() : answers.front();
    const std::string got = normalize(input);
    if (got.empty()) {
        result.empty = true;
        return result;
    }
    for (const std::string &answer : answers) {
        if (normalize(answer) == got) {
            result.ok = true;
            result.expected = answer;
            return result;
        }
    }
    const std::string bare = stripAccents(got);
    for (const std::string &answer : answers) {
        if (stripAccents(normalize(answer)) == bare) {
            result.ok = true;
            result.almost = true;
            result.expected = answer;
            return result;
        }
    }
    return result;
}

std::string cutText(std::string_view text, std::size_t characters)
{
    std::size_t count = 0;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if ((static_cast<unsigned char>(text[i]) & 0xC0) != 0x80 && count++ == characters)
            return std::string(text.substr(0, i));
    }
    return std::string(text);
}

} // namespace lq
