#pragma once

#include <string>
#include <string_view>
#include <vector>

// Answer checking, ported from LQ.util (app/js/core/util.js). Text is UTF-8; the letters Spanish uses
// (the Latin-1 range) are lowercased and stripped of accents, which is all the course needs.
namespace lq {

std::string toLower(std::string_view text);
// Lowercase, punctuation as spaces, single spaces, trimmed: case and punctuation never matter.
std::string normalize(std::string_view text);
// á → a, ñ → n, ü → u…: what is left once the accents are removed.
std::string stripAccents(std::string_view text);
std::vector<std::string> words(std::string_view text);
std::size_t codePointCount(std::string_view text);
// The first `characters` code points of UTF-8 text, never cutting one in half.
std::string cutText(std::string_view text, std::size_t characters);

struct AnswerCheck {
    bool ok = false;
    bool almost = false;  // right except for the accents: accepted, with a hint
    bool empty = false;
    std::string expected;  // the accepted answer that matched, or the first one
};

AnswerCheck checkAnswer(std::string_view input, const std::vector<std::string> &answers);

} // namespace lq
