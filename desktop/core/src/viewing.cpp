#include "lq/viewing.h"

#include "lq/text.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <charconv>
#include <map>
#include <tuple>

namespace lq {

namespace {

constexpr std::string_view kSourceSeparator = " · ";

// RFC 4180 CSV: quoted fields may hold commas, line breaks and doubled quotes; lines end in \n or \r\n.
std::vector<std::vector<std::string>> parseCsv(std::string_view text)
{
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string field;
    bool quoted = false;
    bool any = false;  // the row has at least one character or separator
    for (std::size_t i = 0; i < text.size(); ++i) {
        const char c = text[i];
        if (quoted) {
            if (c == '"') {
                if (i + 1 < text.size() && text[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                field += c;
            }
            continue;
        }
        if (c == '"') {
            quoted = true;
            any = true;
        } else if (c == ',') {
            row.push_back(std::move(field));
            field.clear();
            any = true;
        } else if (c == '\n' || c == '\r') {
            if (c == '\r' && i + 1 < text.size() && text[i + 1] == '\n')
                ++i;
            if (any || !field.empty()) {
                row.push_back(std::move(field));
                rows.push_back(std::move(row));
            }
            row.clear();
            field.clear();
            any = false;
        } else {
            field += c;
            any = true;
        }
    }
    if (any || !field.empty()) {
        row.push_back(std::move(field));
        rows.push_back(std::move(row));
    }
    return rows;
}

std::string trim(std::string_view text)
{
    constexpr std::string_view space = " \t\r\n";
    const auto first = text.find_first_not_of(space);
    if (first == std::string_view::npos)
        return {};
    return std::string(text.substr(first, text.find_last_not_of(space) - first + 1));
}

std::string lower(std::string_view text)
{
    std::string out(text);
    std::ranges::transform(out, out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

// The digits of text as a number; nullopt for anything else.
std::optional<int> number(std::string_view text)
{
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size() || text.empty())
        return std::nullopt;
    return value;
}

// "3/4/26", "03.04.2026", "2026-04-03" or "2026-04-03 20:13:54" as three numbers in the order written.
std::optional<std::array<int, 3>> dateParts(std::string_view text)
{
    const std::string date = trim(text.substr(0, text.find(' ')));
    std::array<int, 3> parts{};
    std::size_t start = 0;
    for (int i = 0; i < 3; ++i) {
        const std::size_t end = i < 2 ? date.find_first_of("/.-", start) : date.size();
        if (end == std::string::npos)
            return std::nullopt;
        const auto value = number(std::string_view(date).substr(start, end - start));
        if (!value)
            return std::nullopt;
        parts[static_cast<std::size_t>(i)] = *value;
        start = end + 1;
    }
    return parts;
}

std::optional<Date> toDate(int year, int month, int day)
{
    if (year < 100)
        year += 2000;
    if (year < 1990 || year > 2200 || month < 1 || month > 12 || day < 1 || day > 31)
        return std::nullopt;
    // parseIso turns down a day the month does not have, such as 31 February.
    const auto two = [](int n) { return (n < 10 ? "0" : "") + std::to_string(n); };
    return parseIso(std::to_string(year) + "-" + two(month) + "-" + two(day));
}

std::optional<Date> dateOf(const std::array<int, 3> &p, bool dayFirst)
{
    if (p[0] >= 1000)
        return toDate(p[0], p[1], p[2]);
    return dayFirst ? toDate(p[2], p[1], p[0]) : toDate(p[2], p[0], p[1]);
}

// "00:42:10" or "42:10" as seconds; a plain number such as "42" is minutes.
std::optional<int> secondsOf(std::string_view text)
{
    const std::string value = trim(text);
    if (value.empty())
        return std::nullopt;
    int seconds = 0;
    std::size_t start = 0;
    int fields = 0;
    while (start <= value.size()) {
        const std::size_t end = std::min(value.find(':', start), value.size());
        const auto part = number(std::string_view(value).substr(start, end - start));
        if (!part || *part < 0)
            return std::nullopt;
        seconds = seconds * 60 + *part;
        ++fields;
        start = end + 1;
        if (end == value.size())
            break;
    }
    return fields == 1 ? seconds * 60 : seconds;
}

int column(const std::vector<std::string> &header, std::initializer_list<std::string_view> names)
{
    for (std::size_t i = 0; i < header.size(); ++i) {
        const std::string name = lower(trim(header[i]));
        for (const std::string_view wanted : names) {
            if (name == wanted)
                return static_cast<int>(i);
        }
    }
    return -1;
}

std::string cell(const std::vector<std::string> &row, int index)
{
    return index >= 0 && static_cast<std::size_t>(index) < row.size() ? trim(row[static_cast<std::size_t>(index)])
                                                                        : std::string();
}

} // namespace

std::string showOf(std::string_view title)
{
    const std::size_t first = title.find(": ");
    if (first == std::string_view::npos || title.find(": ", first + 2) == std::string_view::npos)
        return trim(title);
    return trim(title.substr(0, first));
}

bool madeInSpanish(std::string_view show)
{
    // Series and films made in Spanish that Netflix carries or has carried, as normalize() and stripAccents()
    // leave their titles. English titles too common to tell apart ("The Kingdom", "Champions") are left out.
    static const std::set<std::string, std::less<>> kMadeInSpanish{
        // Spain
        "la casa de papel", "money heist", "elite", "las chicas del cable", "cable girls", "alta mar", "high seas",
        "vis a vis", "locked up", "el ministerio del tiempo", "the ministry of time", "paquita salas", "valeria",
        "aqui no hay quien viva", "la que se avecina", "con el culo al aire", "sky rojo", "el inocente", "the innocent", "berlin", "toy boy", "los favoritos de midas",
        "the minions of midas", "hache", "farina", "cocaine coast", "gran hotel", "velvet", "el internado",
        "the boarding school", "merli", "machos alfa", "alpha males", "bienvenidos a eden", "welcome to eden",
        "el cuerpo en llamas", "burning body", "las de la ultima fila", "the girls at the back", "fisica o quimica",
        "el hoyo", "the platform", "contratiempo", "the invisible guest", "la sociedad de la nieve",
        "society of the snow", "ocho apellidos vascos", "spanish affair", "veneno", "patria",
        // Mexico
        "club de cuervos", "club of crows", "la casa de las flores", "the house of flowers", "oscuro deseo",
        "dark desire", "quien mato a sara", "who killed sara", "luis miguel", "control z", "monarca", "rebelde",
        "ingobernable", "roma", "bardo", "pedro paramo", "el chapo", "diablero", "la reina del sur",
        "el senor de los cielos", "rubi", "la rosa de guadalupe",
        // Colombia, Argentina, Chile
        "cafe con aroma de mujer", "pablo escobar el patron del mal", "pablo escobar the drug lord",
        "la reina del flow", "queen of the flow", "siempre bruja", "always a witch", "frontera verde",
        "green frontier", "cien anos de soledad", "one hundred years of solitude", "distrito salvaje",
        "wild district", "yo soy betty la fea", "el marginal", "puerta 7", "okupas", "el eternauta",
        "the eternaut", "relatos salvajes", "wild tales", "el secreto de sus ojos", "42 dias en la oscuridad",
        "42 days of darkness",
    };
    return kMadeInSpanish.contains(stripAccents(normalize(show)));
}

std::string viewingSource(std::string_view service, std::string_view title)
{
    return cutText(std::string(service) + std::string(kSourceSeparator) + std::string(title), kMaxLogText);
}

std::optional<ViewingFile> readViewing(std::string_view text, bool dayFirst, std::string *why)
{
    const auto fail = [why](std::string reason) -> std::optional<ViewingFile> {
        if (why)
            *why = std::move(reason);
        return std::nullopt;
    };
    if (text.size() > kMaxViewingBytes)
        return fail("This file is too large to be a viewing history.");
    if (text.starts_with("\xEF\xBB\xBF"))
        text.remove_prefix(3);
    const auto rows = parseCsv(text);
    if (rows.empty())
        return fail("This file is empty.");

    const auto &header = rows.front();
    const int title = column(header, {"title"});
    const int date = column(header, {"date", "start time", "watched at", "watched on"});
    const int duration = column(header, {"duration", "minutes"});
    const int profile = column(header, {"profile name"});
    const int extra = column(header, {"supplemental video type"});
    if (title < 0 || date < 0)
        return fail("This is not a viewing history LearnQuick can read: it needs a Title and a Date column, like "
                    "the file Netflix gives you under Account → Viewing activity → Download all.");

    ViewingFile file;
    file.service = profile >= 0 || (header.size() == 2 && duration < 0) ? "Netflix" : "TV";
    file.hasMinutes = duration >= 0;

    // A date such as 3/4/26 is read day-first if any date in the file only makes sense that way, month-first if
    // any only makes sense the other way, and as the system's locale writes dates when none settles it.
    bool sawDayFirst = false;
    bool sawMonthFirst = false;
    for (std::size_t r = 1; r < rows.size(); ++r) {
        if (const auto p = dateParts(cell(rows[r], date)); p && (*p)[0] < 1000) {
            sawDayFirst = sawDayFirst || (*p)[0] > 12;
            sawMonthFirst = sawMonthFirst || (*p)[1] > 12;
        }
    }
    const bool readDayFirst = sawDayFirst != sawMonthFirst ? sawDayFirst : dayFirst;

    for (std::size_t r = 1; r < rows.size(); ++r) {
        const auto &row = rows[r];
        Viewing v;
        v.title = cell(row, title);
        const auto parts = dateParts(cell(row, date));
        const auto day = parts ? dateOf(*parts, readDayFirst) : std::nullopt;
        if (v.title.empty() || !day || !cell(row, extra).empty()) {
            ++file.skipped;
            continue;
        }
        v.date = *day;
        v.show = showOf(v.title);
        v.profile = cell(row, profile);
        if (file.hasMinutes) {
            const auto seconds = secondsOf(cell(row, duration));
            if (!seconds || *seconds < kMinViewingMinutes * 60) {
                ++file.skipped;
                continue;
            }
            v.minutes = (*seconds + 30) / 60;
        }
        file.items.push_back(std::move(v));
    }
    if (file.items.empty())
        return fail("This viewing history has nothing LearnQuick can count: no row with a title and a date"
                    + std::string(file.hasMinutes ? " that was played for 2 minutes or more." : "."));
    return file;
}

std::set<std::string> importedShows(const LearnerState &state, std::string_view service)
{
    const std::string prefix = std::string(service) + std::string(kSourceSeparator);
    std::set<std::string> shows;
    for (const InputEntry &entry : state.input) {
        if (entry.imported && entry.source.starts_with(prefix))
            shows.insert(showOf(std::string_view(entry.source).substr(prefix.size())));
    }
    return shows;
}

ViewingImport importViewing(LearnerState &state, const ViewingFile &file, const std::set<std::string> &shows,
                            std::string_view profile, int minutesPerItem, Date from, Date today)
{
    // One entry per title and day: a Netflix export lists each time an episode was resumed.
    std::map<std::tuple<Date, std::string>, int> watched;
    for (const Viewing &v : file.items) {
        if (v.date > today || v.date < from || !shows.contains(v.show) || (!profile.empty() && v.profile != profile))
            continue;
        watched[{v.date, v.title}] += file.hasMinutes ? v.minutes : 0;
    }

    std::set<std::tuple<Date, std::string>> before;
    for (const InputEntry &entry : state.input) {
        if (entry.imported)
            before.insert({entry.date, entry.source});
    }

    ViewingImport result;
    for (const auto &[key, total] : watched) {
        const auto &[date, title] = key;
        std::string source = viewingSource(file.service, title);
        if (before.contains({date, source})) {
            ++result.already;
            continue;
        }
        const int minutes = std::clamp(file.hasMinutes ? total : minutesPerItem, 1, kMaxLoggedMinutes);
        state.activity[date].inputMinutes += minutes;
        state.input.push_back({date, minutes, "series", std::move(source), true});
        ++result.added;
        result.minutes += minutes;
    }
    // The log reads in the order things happened.
    std::ranges::stable_sort(state.input, {}, &InputEntry::date);
    return result;
}

} // namespace lq
