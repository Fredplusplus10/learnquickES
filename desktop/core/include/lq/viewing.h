#pragma once

#include "lq/date.h"
#include "lq/progress.h"

#include <cstddef>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

// What the learner watched, from a streaming service's own export of their viewing history. No service offers
// an API to read it (Netflix closed its public one in 2014; HBO Max, Prime Video and Disney+ never had one), and
// asking for a password would hand the app the whole account, so the learner downloads the file and opens it
// here. Nothing leaves the computer.
//
// Read: Netflix's "Download all" from Viewing activity (Title, Date), the ViewingActivity.csv of Netflix's
// personal-information download (with profile, start time and duration), and any CSV with Title and Date
// columns and optionally Minutes, so other services' exports can be shaped to fit.
//
// No file says which audio language was on, so the learner picks the shows they watched in Spanish.
namespace lq {

struct Viewing {
    std::string title;    // as the service writes it: "La casa de papel: Parte 1: Episodio 1"
    std::string show;     // the series, or a film's title
    std::string profile;  // the Netflix profile; empty when the file has none
    Date date{};
    int minutes = 0;  // 0 when the file has no durations
    bool operator==(const Viewing &) const = default;
};

struct ViewingFile {
    std::string service;  // "Netflix", or "TV" for a file of another shape
    bool hasMinutes = false;
    std::vector<Viewing> items;
    int skipped = 0;  // trailers, previews and rows without a title or a readable date
};

inline constexpr std::size_t kMaxViewingBytes = 32u * 1024u * 1024u;
// Shorter plays are previews, trailers and autoplay.
inline constexpr int kMinViewingMinutes = 2;

// nullopt, with the reason in *why, when the text is not a viewing history LearnQuick can read. dayFirst says how
// to read a date such as 3/4/26 when no date in the file settles it (from the system's locale).
std::optional<ViewingFile> readViewing(std::string_view text, bool dayFirst, std::string *why = nullptr);

// "La casa de papel: Parte 1: Episodio 1" → "La casa de papel". A title with fewer than two ": " is a film or a
// one-off and is its own show.
std::string showOf(std::string_view title);

// Whether a show or film was made in Spanish, by its Spanish or English title on Netflix: "La casa de papel" and
// "Money Heist" both are. A hint, never a tick: Netflix plays the dub in the profile's language by default, so only
// the learner knows which audio was on. Accents, case and punctuation don't matter.
bool madeInSpanish(std::string_view show);

// How an imported entry names what was watched in the immersion log: "Netflix · <title>".
std::string viewingSource(std::string_view service, std::string_view title);

// The shows already imported from this service before, from the log's own entries.
std::set<std::string> importedShows(const LearnerState &state, std::string_view service);

struct ViewingImport {
    int added = 0;    // entries added to the log
    int minutes = 0;  // their minutes
    int already = 0;  // titles and days that were imported before
};

// Adds what was watched of the chosen shows to the immersion log as "series", one entry per title and day, on the
// day it was watched and without XP (InputEntry::imported). profile limits it to one Netflix profile (empty for
// all). minutesPerItem stands in for each title when the file has no durations. Only days from `from` to today
// count, and a title and day imported before is not added twice.
ViewingImport importViewing(LearnerState &state, const ViewingFile &file, const std::set<std::string> &shows,
                            std::string_view profile, int minutesPerItem, Date from, Date today);

} // namespace lq
