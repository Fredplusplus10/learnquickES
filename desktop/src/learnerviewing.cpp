// Listening read from a streaming service's viewing history (lq/viewing.h): open the file the service lets the
// learner download, pick the shows watched in Spanish, add them to the immersion log.

#include "learnercontroller.h"
#include "qtdate.h"

#include <QDir>
#include <QFile>
#include <QLocale>
#include <QStandardPaths>

#include <algorithm>
#include <map>
#include <set>
#include <tuple>

namespace {

QString qs(std::string_view s)
{
    return QString::fromUtf8(s.data(), static_cast<qsizetype>(s.size()));
}

QString longDate(lq::Date date)
{
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(toQDate(date), QStringLiteral("MMM d, yyyy"));
}

QVariantMap failure(const QString &why)
{
    return {{QStringLiteral("error"), why}};
}

// 3/4/26 is read as the computer writes short dates, unless the file's own dates settle it.
bool systemDayFirst()
{
    return !QLocale::system().dateFormat(QLocale::ShortFormat).startsWith(QLatin1Char('M'));
}

std::optional<lq::ViewingFile> readFile(const QString &path, QString *why)
{
    QFile in(path);
    if (!in.open(QIODevice::ReadOnly)) {
        *why = QStringLiteral("%1 could not be opened: %2").arg(QFileInfo(path).fileName(), in.errorString());
        return std::nullopt;
    }
    // size() is 0 for a pipe or a device, so the limit is also kept while reading: one byte past it is enough
    // to know. Every file within the limit is read whole, as before.
    const qint64 limit = static_cast<qint64>(lq::kMaxViewingBytes);
    const QByteArray text = in.size() > limit ? QByteArray() : in.read(limit + 1);
    if (in.size() > limit || text.size() > limit) {
        *why = QStringLiteral("This file is too large to be a viewing history.");
        return std::nullopt;
    }
    std::string reason;
    auto viewing = lq::readViewing(std::string_view(text.constData(), static_cast<std::size_t>(text.size())),
                                   systemDayFirst(), &reason);
    if (!viewing)
        *why = qs(reason);
    return viewing;
}

// How long a history in Downloads is offered: long enough to come back to it, not so long it nags.
constexpr qint64 kDownloadDays = 14;

} // namespace

QVariantMap LearnerController::readViewing(const QUrl &file)
{
    m_viewing.reset();
    if (!m_state)
        return failure(QStringLiteral("Your learner data is not available."));
    QString why;
    auto viewing = readFile(file.toLocalFile(), &why);
    if (!viewing)
        return failure(why);

    const lq::Date start = m_state->plan.start;
    const std::set<std::string> before = lq::importedShows(*m_state, viewing->service);
    struct Show {
        std::set<std::string> titles;
        int minutes = 0;
        int sinceStart = 0;  // titles and days on or after the plan's start
        lq::Date first = lq::Date::max();
        lq::Date last = lq::Date::min();
    };
    std::map<std::string, Show> shows;
    std::set<std::string> profiles;
    lq::Date first = lq::Date::max();
    lq::Date last = lq::Date::min();
    for (const lq::Viewing &v : viewing->items) {
        Show &show = shows[v.show];
        show.titles.insert(v.title);
        show.minutes += v.minutes;
        show.sinceStart += v.date >= start ? 1 : 0;
        show.first = std::min(show.first, v.date);
        show.last = std::max(show.last, v.date);
        first = std::min(first, v.date);
        last = std::max(last, v.date);
        if (!v.profile.empty())
            profiles.insert(v.profile);
    }

    QVariantList list;
    for (const auto &[name, show] : shows) {
        list.append(QVariantMap{
            {QStringLiteral("show"), qs(name)},
            {QStringLiteral("titles"), static_cast<int>(show.titles.size())},
            {QStringLiteral("minutes"), show.minutes},
            {QStringLiteral("sinceStart"), show.sinceStart},
            {QStringLiteral("first"), longDate(show.first)},
            {QStringLiteral("last"), longDate(show.last)},
            {QStringLiteral("before"), before.contains(name)},
            {QStringLiteral("spanish"), lq::madeInSpanish(name)},
        });
    }
    // Shows imported before first, then those made in Spanish, then the most watched: where the Spanish is likely.
    std::ranges::stable_sort(list, [](const QVariant &a, const QVariant &b) {
        const auto key = [](const QVariant &v) {
            const QVariantMap m = v.toMap();
            return std::tuple(m.value(QStringLiteral("before")).toBool(), m.value(QStringLiteral("spanish")).toBool(),
                              m.value(QStringLiteral("titles")).toInt());
        };
        return key(a) > key(b);
    });
    QStringList profileList;
    for (const std::string &profile : profiles)
        profileList.append(qs(profile));

    const QVariantMap result{
        {QStringLiteral("service"), qs(viewing->service)},
        {QStringLiteral("hasMinutes"), viewing->hasMinutes},
        {QStringLiteral("rows"), static_cast<int>(viewing->items.size())},
        {QStringLiteral("skipped"), viewing->skipped},
        {QStringLiteral("first"), longDate(first)},
        {QStringLiteral("last"), longDate(last)},
        {QStringLiteral("planStart"), longDate(start)},
        {QStringLiteral("profiles"), profileList},
        {QStringLiteral("shows"), list},
    };
    m_viewing = std::move(viewing);
    return result;
}

QVariantMap LearnerController::importViewing(const QStringList &shows, const QString &profile, int minutesPerItem,
                                             bool beforeStart)
{
    if (!m_viewing || !m_state)
        return failure(QStringLiteral("Open a viewing history first."));
    std::set<std::string> chosen;
    for (const QString &show : shows)
        chosen.insert(show.toStdString());
    lq::ViewingImport done;
    const lq::Date from = beforeStart ? lq::Date::min() : m_state->plan.start;
    apply([&](lq::LearnerState &state) {
        done = lq::importViewing(state, *m_viewing, chosen, profile.toStdString(), minutesPerItem, from, day());
        return lq::Events{};
    });
    m_viewing.reset();
    return {
        {QStringLiteral("added"), done.added},
        {QStringLiteral("minutes"), done.minutes},
        {QStringLiteral("already"), done.already},
    };
}

QVariantMap LearnerController::downloadedViewing() const
{
    return downloadedViewingIn(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
                               QDateTime::currentDateTime());
}

QVariantMap LearnerController::downloadedViewingIn(const QString &folder, const QDateTime &now) const
{
    if (!m_state || folder.isEmpty())
        return {};
    // Netflix's "Download all" (a second download gets " (1)" from the browser), and ViewingActivity.csv in the
    // unzipped personal-information download.
    const QDir dir(folder);
    QFileInfoList found = dir.entryInfoList({QStringLiteral("NetflixViewingHistory*.csv")}, QDir::Files);
    // Unzipping can add a folder of the same name inside, so one level down is looked at too.
    for (const QFileInfo &report : dir.entryInfoList({QStringLiteral("netflix-report*")}, QDir::Dirs)) {
        QStringList roots{report.filePath()};
        for (const QFileInfo &inner : QDir(report.filePath()).entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
            roots.append(inner.filePath());
        for (const QString &root : roots) {
            const QFileInfo activity(root + QStringLiteral("/CONTENT_INTERACTION/ViewingActivity.csv"));
            if (activity.isFile())
                found.append(activity);
        }
    }
    std::sort(found.begin(), found.end(),
              [](const QFileInfo &a, const QFileInfo &b) { return a.lastModified() > b.lastModified(); });
    if (found.isEmpty() || found.front().lastModified().daysTo(now) > kDownloadDays)
        return {};

    // Offered only while it holds days after the last viewing imported, so an imported file stops asking.
    QString why;
    const auto viewing = readFile(found.front().filePath(), &why);
    if (!viewing)
        return {};
    lq::Date imported = lq::Date::min();
    for (const lq::InputEntry &entry : m_state->input) {
        if (entry.imported)
            imported = std::max(imported, entry.date);
    }
    const bool fresh = std::ranges::any_of(viewing->items, [imported](const lq::Viewing &v) { return v.date > imported; });
    if (!fresh)
        return {};

    const QDateTime saved = found.front().lastModified();
    const QLocale english(QLocale::English, QLocale::UnitedStates);
    const qint64 days = saved.date().daysTo(now.date());
    const QString when = days == 0   ? QStringLiteral("today at ") + english.toString(saved.time(), QStringLiteral("h:mm AP"))
                         : days == 1 ? QStringLiteral("yesterday")
                                     : QStringLiteral("on ") + english.toString(saved.date(), QStringLiteral("MMM d"));
    return {
        {QStringLiteral("url"), QUrl::fromLocalFile(found.front().filePath())},
        {QStringLiteral("name"), found.front().fileName()},
        {QStringLiteral("saved"), when},
    };
}
