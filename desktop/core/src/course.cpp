#include "lq/course.h"

#include <nlohmann/json.hpp>

namespace lq {

namespace {

using nlohmann::json;

std::vector<std::string> strings(const json &list)
{
    return list.get<std::vector<std::string>>();
}

std::pair<std::string, std::string> pairOf(const json &item)
{
    return {item.at(0).get<std::string>(), item.at(1).get<std::string>()};
}

Unit parseUnit(const json &u)
{
    Unit unit;
    unit.id = u.at("id").get<std::string>();
    unit.n = u.at("n").get<int>();
    unit.phase = u.at("phase").get<int>();
    unit.title = u.at("title").get<std::string>();
    unit.titleEs = u.at("es").get<std::string>();
    unit.canDo = u.at("canDo").get<std::string>();
    unit.trick = u.at("trick").get<int>();
    unit.art = u.at("art").get<std::string>();
    unit.coach = u.at("coach").get<std::string>();
    for (const json &v : u.at("vocab")) {
        const std::string g = v.at("g").get<std::string>();
        unit.vocab.push_back({v.at("id").get<std::string>(), v.at("es").get<std::string>(), v.at("en").get<std::string>(),
                              v.at("ex").get<std::string>(), g.empty() ? '\0' : g.front()});
    }
    const json &g = u.at("grammar");
    unit.grammar.title = g.at("title").get<std::string>();
    unit.grammar.hack = g.at("hack").get<std::string>();
    unit.grammar.tip = g.at("tip").get<std::string>();
    for (const json &t : g.at("tables"))
        unit.grammar.tables.push_back({strings(t.at("head")), t.at("rows").get<std::vector<std::vector<std::string>>>()});
    for (const json &e : g.at("examples"))
        unit.grammar.examples.push_back(pairOf(e));
    for (const json &line : u.at("dialogue"))
        unit.dialogue.push_back({line.at(0).get<std::string>(), line.at(1).get<std::string>(), line.at(2).get<std::string>()});
    for (const json &e : u.at("ex")) {
        unit.exercises.push_back({e.at("t").get<std::string>(), e.at("q").get<std::string>(), strings(e.at("o")),
                                  strings(e.at("a")), e.at("hint").get<std::string>(), e.at("why").get<std::string>(),
                                  e.at("es").get<std::string>(), e.at("en").get<std::string>()});
    }
    unit.speak = strings(u.at("speak"));
    return unit;
}

MockPart parseMockPart(const json &part)
{
    MockPart out;
    out.text = part.at("text").get<std::string>();
    for (const json &q : part.at("questions"))
        out.questions.push_back({q.at("q").get<std::string>(), strings(q.at("o"))});
    return out;
}

} // namespace

const Vocab *Course::word(std::string_view id, int *unit) const
{
    const auto it = index.find(id);
    if (it == index.end())
        return nullptr;
    if (unit)
        *unit = it->second.first;
    return &units[static_cast<std::size_t>(it->second.first)].vocab[it->second.second];
}

std::vector<std::pair<int, const Vocab *>> Course::allWords() const
{
    std::vector<std::pair<int, const Vocab *>> out;
    for (std::size_t u = 0; u < units.size(); ++u) {
        for (std::size_t i = 0; i < units[u].vocab.size(); ++i) {
            const auto it = index.find(units[u].vocab[i].id);
            if (it != index.end() && it->second == std::pair<int, std::size_t>(static_cast<int>(u), i))
                out.emplace_back(static_cast<int>(u), &units[u].vocab[i]);
        }
    }
    return out;
}

std::vector<std::vector<std::string>> Course::unitWordIds() const
{
    std::vector<std::vector<std::string>> out;
    for (const Unit &unit : units) {
        std::vector<std::string> ids;
        for (const Vocab &v : unit.vocab)
            ids.push_back(v.id);
        out.push_back(std::move(ids));
    }
    return out;
}

std::optional<Course> parseCourse(std::string_view text, std::string *error)
{
    try {
        const json doc = json::parse(text);
        Course course;
        for (const json &p : doc.at("phases")) {
            course.phases.push_back({p.at("n").get<int>(), p.at("en").get<std::string>(), p.at("es").get<std::string>(),
                                     p.at("desc").get<std::string>()});
        }
        for (const json &u : doc.at("units"))
            course.units.push_back(parseUnit(u));

        const json &practice = doc.at("practice");
        for (const auto &[key, reason] : practice.at("pastReasons").items())
            course.pastReasons.emplace(key, reason.get<std::string>());
        for (const json &p : practice.at("past")) {
            course.past.push_back({p.at(0).get<std::string>(), p.at(1).get<std::string>(), p.at(2).get<std::string>(),
                                   p.at(3).get<std::string>()});
        }
        for (const json &r : practice.at("cognateRules")) {
            CognateRule rule{r.at(0).get<std::string>(), r.at(1).get<std::string>(), {}};
            for (const json &pair : r.at(2))
                rule.examples.push_back(pairOf(pair));
            course.cognateRules.push_back(std::move(rule));
        }
        for (const json &f : practice.at("falseFriends")) {
            course.falseFriends.push_back({f.at(0).get<std::string>(), f.at(1).get<std::string>(), f.at(2).get<std::string>(),
                                           f.at(3).get<std::string>()});
        }
        for (const json &r : practice.at("genderRules"))
            course.genderRules.push_back({r.at(0).get<std::string>(), r.at(1).get<std::string>(), r.at(2).get<std::string>()});
        for (const json &w : practice.at("genderWords"))
            course.genderWords.push_back({w.at(0).get<std::string>(), w.at(1).get<std::string>(), w.at(2).get<std::string>()});
        for (const json &g : practice.at("pronunciation"))
            course.pronunciation.push_back({g.at("title").get<std::string>(), g.at("tip").get<std::string>(), strings(g.at("items"))});
        course.describeFrames = strings(practice.at("describeFrames"));
        for (const json &d : practice.at("describe"))
            course.describe.push_back({d.at(0).get<std::string>(), d.at(1).get<std::string>(), d.at(2).get<std::string>()});

        const json &mock = doc.at("mock");
        course.mock.reading = parseMockPart(mock.at("reading"));
        course.mock.listening = parseMockPart(mock.at("listening"));
        course.mock.writingPrompt = mock.at("writing").at("prompt").get<std::string>();
        for (const json &t : mock.at("speaking"))
            course.mock.speaking.push_back({t.at("title").get<std::string>(), t.at("prompt").get<std::string>(), t.value("image", "")});

        const json &immersion = doc.at("immersion");
        for (const json &r : immersion.at("resources")) {
            course.immersion.resources.push_back({r.at("name").get<std::string>(), r.at("level").get<std::string>(),
                                                  r.at("accent").get<std::string>(), r.at("kind").get<std::string>(),
                                                  r.at("url").get<std::string>(), r.at("note").get<std::string>()});
        }
        for (const json &x : immersion.at("series")) {
            Series series{x.at("name").get<std::string>(), x.at("level").get<std::string>(), x.at("years").get<std::string>(),
                          x.at("kind").get<std::string>(), x.at("note").get<std::string>(), {}};
            for (const json &w : x.at("where"))
                series.where.push_back({w.at("service").get<std::string>(), w.at("detail").get<std::string>()});
            course.immersion.series.push_back(std::move(series));
        }
        course.immersion.seriesWhere = immersion.at("seriesWhere").get<std::string>();
        course.immersion.seriesTips = strings(immersion.at("seriesTips"));
        for (const json &m : immersion.at("missions"))
            course.immersion.missions.push_back({m.at("id").get<std::string>(), m.at("title").get<std::string>(), m.at("how").get<std::string>()});
        for (const json &p : immersion.at("exchangePhrases"))
            course.immersion.exchangePhrases.push_back(pairOf(p));

        if (course.phases.size() != 5 || course.units.size() != 25) {
            *error = "the course should have 5 phases and 25 units";
            return std::nullopt;
        }
        for (std::size_t u = 0; u < course.units.size(); ++u) {
            for (std::size_t i = 0; i < course.units[u].vocab.size(); ++i)
                course.index.emplace(course.units[u].vocab[i].id, std::pair<int, std::size_t>(static_cast<int>(u), i));
        }
        return course;
    } catch (const nlohmann::json::exception &e) {
        *error = e.what();
        return std::nullopt;
    }
}

} // namespace lq
