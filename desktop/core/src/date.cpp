#include "lq/date.h"

#include <charconv>
#include <cstdio>

namespace lq {

using namespace std::chrono;

Date makeDate(int y, unsigned m, unsigned d)
{
    return sys_days{year{y} / month{m} / day{d}};
}

std::optional<Date> parseIso(std::string_view text)
{
    if (text.size() != 10 || text[4] != '-' || text[7] != '-')
        return std::nullopt;
    const auto field = [text](std::size_t pos, std::size_t len, auto &out) {
        const char *begin = text.data() + pos;
        const auto [end, error] = std::from_chars(begin, begin + len, out);
        return error == std::errc{} && end == begin + len;
    };
    int y = 0;
    unsigned m = 0, d = 0;
    if (!field(0, 4, y) || !field(5, 2, m) || !field(8, 2, d))
        return std::nullopt;
    const year_month_day ymd{year{y}, month{m}, day{d}};
    if (!ymd.ok())
        return std::nullopt;
    return sys_days{ymd};
}

std::string toIso(Date date)
{
    const year_month_day ymd{date};
    char buffer[16];
    std::snprintf(buffer, sizeof buffer, "%04d-%02u-%02u", static_cast<int>(ymd.year()),
                  static_cast<unsigned>(ymd.month()), static_cast<unsigned>(ymd.day()));
    return buffer;
}

int weekdayOf(Date date)
{
    return static_cast<int>(weekday{date}.c_encoding());
}

Date weekStart(Date date)
{
    const int wd = weekdayOf(date);
    return addDays(date, wd == 0 ? -6 : 1 - wd);
}

int yearOf(Date date)
{
    return static_cast<int>(year_month_day{date}.year());
}

} // namespace lq
