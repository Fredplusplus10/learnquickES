#pragma once

#include "lq/course.h"
#include "lq/date.h"

#include <catch2/catch_tostring.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

// Readable dates in assertion failures ("2026-11-02" instead of a day count).
namespace Catch {
template <>
struct StringMaker<lq::Date> {
    static std::string convert(lq::Date date) { return lq::toIso(date); }
};
} // namespace Catch

namespace lq::test {

inline Date d(const char *iso)
{
    const auto date = parseIso(iso);
    if (!date)
        throw std::invalid_argument(std::string("bad test date: ") + iso);
    return *date;
}

inline std::string readFile(const std::string &path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in)
        throw std::runtime_error("cannot open " + path);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

// The exported course (content/course.json), parsed once by the engine's own loader.
inline const Course &course()
{
    static const Course parsed = [] {
        std::string error;
        auto loaded = parseCourse(readFile(LQ_CONTENT_DIR "/course.json"), &error);
        if (!loaded)
            throw std::runtime_error("course.json: " + error);
        return *std::move(loaded);
    }();
    return parsed;
}

} // namespace lq::test
