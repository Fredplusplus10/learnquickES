#include "lq/exercises.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include <numeric>
#include <optional>
#include <set>

namespace lq {

namespace {

// Common words that make plausible wrong tiles in sentence building.
const std::vector<std::string> kFiller{"es",  "está", "muy", "tengo", "el",  "la", "un",  "de",
                                       "que", "pero", "con", "mi",    "su",  "por", "hay", "son"};

bool startsWith(std::string_view s, std::string_view prefix)
{
    return s.substr(0, prefix.size()) == prefix;
}

bool endsWith(std::string_view s, std::string_view suffix)
{
    return s.size() >= suffix.size() && s.substr(s.size() - suffix.size()) == suffix;
}

bool contains(std::string_view s, std::string_view part)
{
    return s.find(part) != std::string_view::npos;
}

// /^(el|la|los|las)\s+(.*)$/i: the article and the rest, when the text starts with one.
std::optional<std::pair<std::string, std::string>> splitArticle(const std::string &es)
{
    const std::string lower = toLower(es);
    for (const std::string_view article : {"el", "la", "los", "las"}) {
        if (!startsWith(lower, article) || lower.size() <= article.size())
            continue;
        std::size_t rest = article.size();
        while (rest < es.size() && (es[rest] == ' ' || es[rest] == '\t'))
            ++rest;
        if (rest == article.size())
            continue;  // "lado" or "lasaña": not an article
        return std::make_pair(std::string(article), es.substr(rest));
    }
    return std::nullopt;
}

std::vector<std::string> splitSpaces(std::string_view text)
{
    std::vector<std::string> out;
    std::size_t start = 0;
    while (start < text.size()) {
        while (start < text.size() && text[start] == ' ')
            ++start;
        const std::size_t end = std::min(text.find(' ', start), text.size());
        if (end > start)
            out.emplace_back(text.substr(start, end - start));
        start = end;
    }
    return out;
}

// Replaces the first "___" in text.
std::string fillBlank(std::string text, std::string_view with)
{
    if (const std::size_t at = text.find("___"); at != std::string::npos)
        text.replace(at, 3, with);
    return text;
}

std::string trim(std::string s)
{
    const auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

const std::vector<Vocab> kNoWords;

} // namespace

// ---- Builders --------------------------------------------------------------------------------------------

Item choiceItem(std::string question, const std::vector<std::string> &options, SeededRandom &rnd)
{
    std::vector<int> order(options.size());
    std::iota(order.begin(), order.end(), 0);
    order = shuffled(std::move(order), rnd);
    Item item;
    item.type = ItemType::Choice;
    item.question = std::move(question);
    for (std::size_t i = 0; i < order.size(); ++i) {
        item.options.push_back(options[static_cast<std::size_t>(order[i])]);
        if (order[i] == 0)
            item.answer = static_cast<int>(i);
    }
    return item;
}

std::vector<std::string> sentenceTiles(std::string_view es, const std::vector<std::string> &pool, SeededRandom &rnd)
{
    std::string spaced(es);
    for (const std::string_view mark : {"¿", "?", "¡", "!", ".", ",", ";", ":"}) {
        for (std::size_t at = spaced.find(mark); at != std::string::npos; at = spaced.find(mark, at))
            spaced.replace(at, mark.size(), " ");
    }
    std::vector<std::string> words = splitSpaces(spaced);
    if (words.empty())
        return {};
    words[0] = toLower(words[0]);  // a capital letter would give the first tile away
    std::set<std::string> lower;
    for (const std::string &w : words)
        lower.insert(toLower(w));
    std::vector<std::string> distractors;
    std::set<std::string> seen;
    for (const std::string &w : pool) {
        if (seen.insert(w).second && !lower.contains(toLower(w)))
            distractors.push_back(w);
    }
    for (std::string &w : sampled(std::move(distractors), 2, rnd))
        words.push_back(std::move(w));
    return shuffled(std::move(words), rnd);
}

std::vector<std::string> vocabPool(const Unit *unit)
{
    std::vector<std::string> pool;
    for (const Vocab &v : unit ? unit->vocab : kNoWords) {
        const auto split = splitArticle(v.es);
        const std::string word = split ? split->second : v.es;
        const bool usable = std::none_of(word.begin(), word.end(), [](char c) { return c == ' ' || c == '\t' || c == '/' || c == '+'; })
                            && !contains(word, "…") && !contains(word, "¿") && !contains(word, "?") && !contains(word, "¡")
                            && !contains(word, "!");
        if (usable)
            pool.push_back(word);
    }
    pool.insert(pool.end(), kFiller.begin(), kFiller.end());
    return pool;
}

Item fromRaw(const RawExercise &raw, const Unit *unit, SeededRandom &rnd)
{
    const std::string skill = raw.type == "listen" ? "listening" : "grammar";
    Item item;
    if (raw.type == "mc") {
        item = choiceItem(raw.question, raw.options, rnd);
        item.why = raw.why;
    } else if (raw.type == "fill") {
        item.type = ItemType::Fill;
        item.question = raw.question;
        item.answers = raw.answers;
        item.hint = raw.hint;
        item.why = raw.why;
    } else {
        item.type = raw.type == "listen" ? ItemType::Listen : ItemType::Order;
        item.es = raw.es;
        item.en = raw.en;
        item.tiles = sentenceTiles(raw.es, vocabPool(unit), rnd);
    }
    item.skill = skill;
    item.unit = unit ? unit->n - 1 : -1;
    return item;
}

Item genderItem(const std::string &es, const std::string &en, const std::string &note)
{
    Item item;
    item.type = ItemType::Gender;
    item.skill = "gender";
    const auto split = splitArticle(es);
    item.article = split ? split->first : std::string();
    item.word = split ? split->second : es;
    const bool plural = item.article.size() > 2;
    item.articles = plural ? std::vector<std::string>{"los", "las"} : std::vector<std::string>{"el", "la"};
    item.en = en;
    item.es = es;
    item.note = note;
    return item;
}

Item vocabItem(const Vocab &word, const std::vector<Vocab> &list, SeededRandom &rnd)
{
    std::vector<std::string> others;
    for (const Vocab &v : list) {
        if (v.en != word.en)
            others.push_back(v.en);
    }
    std::vector<std::string> options{word.en};
    for (std::string &o : sampled(std::move(others), 2, rnd))
        options.push_back(std::move(o));
    Item item = choiceItem("¿Qué significa «" + word.es + "»?", options, rnd);
    item.skill = "vocab";
    item.audio = word.es;
    item.why = word.example.empty() ? std::string() : "Ejemplo: " + word.example;
    return item;
}

std::vector<Item> lessonItems(const Course &course, int unitIndex, SeededRandom &rnd)
{
    const Unit &unit = course.units.at(static_cast<std::size_t>(unitIndex));
    std::vector<Item> authored;
    for (const RawExercise &raw : unit.exercises)
        authored.push_back(fromRaw(raw, &unit, rnd));

    std::vector<Vocab> nouns, brief;
    for (const Vocab &v : unit.vocab) {
        if (v.gender && !contains(v.es, "/"))
            nouns.push_back(v);
        if (codePointCount(v.es) < 26)
            brief.push_back(v);
    }
    Item match;
    match.type = ItemType::Match;
    match.skill = "vocab";
    match.unit = unitIndex;
    for (const Vocab &v : sampled(brief, 5, rnd))
        match.pairs.emplace_back(v.es, v.en);

    std::vector<Item> items{match};
    for (const Vocab &v : sampled(unit.vocab, 2, rnd))
        items.push_back(vocabItem(v, unit.vocab, rnd));
    const std::size_t split = std::min<std::size_t>(4, authored.size());
    items.insert(items.end(), authored.begin(), authored.begin() + static_cast<std::ptrdiff_t>(split));
    for (const Vocab &v : sampled(nouns, std::min<std::size_t>(2, nouns.size()), rnd))
        items.push_back(genderItem(v.es, v.en));
    items.insert(items.end(), authored.begin() + static_cast<std::ptrdiff_t>(split), authored.end());
    for (Item &item : items)
        item.unit = unitIndex;
    return items;
}

std::vector<Item> mixedItems(const Course &course, const std::vector<int> &units, int n, SeededRandom &rnd)
{
    const std::vector<int> pool = units.empty() ? std::vector<int>{0} : units;
    std::vector<std::pair<const RawExercise *, const Unit *>> exercises;
    std::vector<Vocab> words;
    for (const int u : pool) {
        const Unit &unit = course.units.at(static_cast<std::size_t>(u));
        for (const RawExercise &raw : unit.exercises)
            exercises.emplace_back(&raw, &unit);
        words.insert(words.end(), unit.vocab.begin(), unit.vocab.end());
    }
    std::vector<Item> items;
    for (const auto &[raw, unit] : sampled(exercises, static_cast<std::size_t>(std::max(0, n - 2)), rnd))
        items.push_back(fromRaw(*raw, unit, rnd));
    for (const Vocab &v : sampled(words, 2, rnd))
        items.push_back(vocabItem(v, words, rnd));
    return shuffled(std::move(items), rnd);
}

std::vector<Item> conjugationItems(const std::vector<std::string_view> &verbList, const std::vector<Tense> &tenses, int n,
                                   bool vosotros, SeededRandom &rnd)
{
    std::vector<int> persons{0, 1, 2, 3, 5};
    if (vosotros)
        persons.push_back(4);
    std::vector<Item> items;
    for (int i = 0; i < n; ++i) {
        Item item;
        item.type = ItemType::Conjugate;
        item.skill = "verbs";
        item.verb = std::string(sampled(verbList, 1, rnd).front());
        item.tense = sampled(tenses, 1, rnd).front();
        item.person = sampled(persons, 1, rnd).front();
        const auto &forms = pronouns(item.person);
        item.pronoun = std::string(sampled(forms, 1, rnd).front());
        item.answers = {conjugate(item.verb, item.tense)[static_cast<std::size_t>(item.person)]};
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> pastItems(const Course &course, int n, SeededRandom &rnd)
{
    std::vector<Item> items;
    for (const PastItem &p : sampled(course.past, static_cast<std::size_t>(n), rnd)) {
        Item item = choiceItem(p.question, {p.right, p.wrong}, rnd);
        item.skill = "past";
        const auto reason = course.pastReasons.find(p.reason);
        item.why = reason == course.pastReasons.end() ? std::string() : reason->second;
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> cognateItems(const Course &course, int n, SeededRandom &rnd)
{
    struct Cognate {
        std::string rule, en, es;
    };
    std::vector<Cognate> all;
    for (const CognateRule &rule : course.cognateRules) {
        for (const auto &[en, es] : rule.examples)
            all.push_back({rule.en + " → " + rule.es, en, es});
    }
    std::vector<Item> items;
    for (const Cognate &c : sampled(all, static_cast<std::size_t>(n), rnd)) {
        Item item;
        item.type = ItemType::Type;
        item.skill = "cognates";
        item.prompt = "Turn this English word into Spanish:";
        item.question = c.en;
        item.answers = {c.es};
        item.why = c.rule;
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> falseFriendItems(const Course &course, int n, SeededRandom &rnd)
{
    std::vector<Item> items;
    for (const FalseFriend &f : sampled(course.falseFriends, static_cast<std::size_t>(n), rnd)) {
        Item item = choiceItem("«" + f.es + "» means…", {f.meaning, f.looksLike}, rnd);
        item.skill = "falsefriends";
        item.audio = f.es;
        item.why = "False friend! \"" + f.looksLike + "\" in Spanish is «" + f.realWord + "».";
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> genderItems(const Course &course, int n, SeededRandom &rnd)
{
    std::vector<GenderWord> courseNouns;
    for (const auto &[unit, word] : course.allWords()) {
        if (word->gender && !contains(word->es, "/"))
            courseNouns.push_back({word->es, word->en, {}});
    }
    std::vector<GenderWord> pool = course.genderWords;
    for (GenderWord &w : sampled(std::move(courseNouns), 15, rnd))
        pool.push_back(std::move(w));
    std::vector<Item> items;
    for (const GenderWord &w : sampled(std::move(pool), static_cast<std::size_t>(n), rnd))
        items.push_back(genderItem(w.es, w.en, w.note));
    return items;
}

std::vector<Item> cardItems(const Course &course, const std::vector<std::string> &ids)
{
    std::vector<Item> items;
    for (const std::string &id : ids) {
        int unit = -1;
        const Vocab *word = course.word(id, &unit);
        if (!word)
            continue;
        Item item;
        item.type = ItemType::Card;
        item.skill = "review";
        item.cardId = id;
        item.es = word->es;
        item.en = word->en;
        item.example = word->example;
        item.gender = word->gender;
        item.unit = unit;
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> speakItems(const std::vector<std::string> &phrases)
{
    std::vector<Item> items;
    for (const std::string &phrase : phrases) {
        Item item;
        item.type = ItemType::Speak;
        item.skill = "speaking";
        item.es = phrase;
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> describeItems(const Course &course, int n, SeededRandom &rnd)
{
    std::vector<Item> items;
    for (const DescribeItem &d : sampled(course.describe, static_cast<std::size_t>(n), rnd)) {
        Item item;
        item.type = ItemType::Describe;
        item.skill = "describe";
        item.en = d.en;
        item.es = d.es;
        item.model = d.model;
        items.push_back(std::move(item));
    }
    return items;
}

int pronunciationGroupFor(const Course &course, Date today)
{
    SeededRandom daily(toIso(today));
    return static_cast<int>(daily.next() * static_cast<double>(course.pronunciation.size()));
}

std::vector<Item> speakingItems(const Course &course, const std::vector<int> &done, Date today, SeededRandom &rnd)
{
    std::vector<std::string> phrases;
    for (const int u : done) {
        const auto &speak = course.units.at(static_cast<std::size_t>(u)).speak;
        phrases.insert(phrases.end(), speak.begin(), speak.end());
    }
    const PronunciationGroup &group = course.pronunciation.at(static_cast<std::size_t>(pronunciationGroupFor(course, today)));
    const std::size_t count = std::min<std::size_t>(4, phrases.size());
    std::vector<std::string> chosen = sampled(std::move(phrases), count, rnd);
    for (std::string &p : sampled(group.items, done.empty() ? 6 : 2, rnd))
        chosen.push_back(std::move(p));
    return speakItems(chosen);
}

std::vector<Item> checkpointItems(const Course &course, int phase, SeededRandom &rnd)
{
    std::vector<std::pair<const RawExercise *, const Unit *>> pool;
    std::vector<Vocab> words;
    for (const Unit &unit : course.units) {
        if (unit.phase != phase)
            continue;
        for (const RawExercise &raw : unit.exercises)
            pool.emplace_back(&raw, &unit);
        words.insert(words.end(), unit.vocab.begin(), unit.vocab.end());
    }
    std::vector<Item> items;
    for (const auto &[raw, unit] : sampled(pool, phase >= 3 ? 8 : 10, rnd))
        items.push_back(fromRaw(*raw, unit, rnd));
    std::vector<Tense> tenses;
    for (const Tense t : kTenses) {
        if (tenseInfo(t).unit < phase * 5)
            tenses.push_back(t);
    }
    for (Item &item : conjugationItems(coreVerbs(), tenses, 3, false, rnd))
        items.push_back(std::move(item));
    for (const Vocab &v : sampled(words, 2, rnd))
        items.push_back(vocabItem(v, words, rnd));
    if (phase >= 3) {
        for (Item &item : pastItems(course, 2, rnd))
            items.push_back(std::move(item));
    }
    return shuffled(std::move(items), rnd);
}

std::vector<Item> mockQuestions(const MockPart &part, const std::string &skill, SeededRandom &rnd)
{
    std::vector<Item> items;
    for (const MockQuestion &q : part.questions) {
        Item item = choiceItem(q.question, q.options, rnd);
        item.skill = skill;
        items.push_back(std::move(item));
    }
    return items;
}

std::vector<Item> mockGrammarItems(const Course &course, SeededRandom &rnd)
{
    std::vector<std::pair<const RawExercise *, const Unit *>> pool;
    for (const Unit &unit : course.units) {
        if (unit.phase < 3)
            continue;
        for (const RawExercise &raw : unit.exercises) {
            if (raw.type == "mc" || raw.type == "fill")
                pool.emplace_back(&raw, &unit);
        }
    }
    std::vector<Item> items;
    for (const auto &[raw, unit] : sampled(pool, 10, rnd))
        items.push_back(fromRaw(*raw, unit, rnd));
    return items;
}

std::vector<int> completedUnits(const LearnerState &state)
{
    std::vector<int> units;
    for (const auto &[unit, score] : state.units)
        units.push_back(unit);
    return units;
}

std::vector<Tense> unlockedTenses(const LearnerState &state)
{
    std::vector<Tense> tenses;
    for (const Tense t : kTenses) {
        const int unit = tenseInfo(t).unit;
        if (unit == 0 || state.units.contains(unit))
            tenses.push_back(t);
    }
    return tenses;
}

std::string_view itemKind(ItemType type)
{
    switch (type) {
    case ItemType::Choice: return "choice";
    case ItemType::Fill: return "fill";
    case ItemType::Type: return "type";
    case ItemType::Conjugate: return "conj";
    case ItemType::Order: return "order";
    case ItemType::Listen: return "listen";
    case ItemType::Match: return "match";
    case ItemType::Gender: return "gender";
    case ItemType::Card: return "card";
    case ItemType::Speak: return "speak";
    case ItemType::Describe: return "describe";
    }
    return {};
}

// ---- Checking --------------------------------------------------------------------------------------------

bool canCheck(const Item &item, const Response &response)
{
    switch (item.type) {
    case ItemType::Choice:
        return response.choice >= 0 && response.choice < static_cast<int>(item.options.size());
    case ItemType::Fill:
    case ItemType::Type:
    case ItemType::Conjugate:
        return !normalize(response.text).empty();
    case ItemType::Order:
    case ItemType::Listen:
        return !response.picked.empty();
    default:
        return false;
    }
}

AnswerCheck checkItem(const Item &item, const Response &response)
{
    switch (item.type) {
    case ItemType::Choice: {
        AnswerCheck result;
        result.ok = response.choice == item.answer;
        result.expected = item.options.at(static_cast<std::size_t>(item.answer));
        return result;
    }
    case ItemType::Order:
    case ItemType::Listen: {
        std::string sentence;
        for (const int i : response.picked) {
            if (i < 0 || i >= static_cast<int>(item.tiles.size()))
                continue;
            if (!sentence.empty())
                sentence += ' ';
            sentence += item.tiles[static_cast<std::size_t>(i)];
        }
        return checkAnswer(sentence, {item.es});
    }
    default:
        return checkAnswer(response.text, item.answers);
    }
}

std::string correctAnswer(const Item &item)
{
    switch (item.type) {
    case ItemType::Choice: return item.options.at(static_cast<std::size_t>(item.answer));
    case ItemType::Gender: return item.article + " " + item.word;
    case ItemType::Match: return {};
    case ItemType::Order:
    case ItemType::Listen: return item.es;
    default: return item.answers.empty() ? std::string() : item.answers.front();
    }
}

std::string sentenceOf(const Item &item)
{
    switch (item.type) {
    case ItemType::Fill:
        return fillBlank(item.question, item.answers.empty() ? std::string() : item.answers.front());
    case ItemType::Choice: {
        if (contains(item.question, "___")) {
            // Options such as "fui … iba" fill several blanks in turn; hints in parentheses are dropped.
            std::string sentence = item.question;
            const std::string &option = item.options.at(static_cast<std::size_t>(item.answer));
            std::size_t start = 0;
            while (true) {
                const std::size_t sep = option.find(" … ", start);
                sentence = fillBlank(sentence, option.substr(start, sep == std::string::npos ? std::string::npos : sep - start));
                if (sep == std::string::npos)
                    break;
                start = sep + std::string_view(" … ").size();
            }
            for (std::size_t open = sentence.find('('); open != std::string::npos; open = sentence.find('(', open)) {
                const std::size_t close = sentence.find(')', open);
                if (close == std::string::npos)
                    break;
                sentence.erase(open, close - open + 1);
            }
            return trim(sentence);
        }
        return item.audio;
    }
    case ItemType::Order:
    case ItemType::Listen: return item.es;
    case ItemType::Conjugate: return item.pronoun + " " + (item.answers.empty() ? std::string() : item.answers.front());
    case ItemType::Gender: return item.es;
    case ItemType::Type: return item.answers.empty() ? std::string() : item.answers.front();
    default: return {};
    }
}

// ---- Session ---------------------------------------------------------------------------------------------

Session::Session(std::vector<Item> items, bool requeue)
    : m_queue(std::move(items))
    , m_firstRound(m_queue.size())
    , m_requeue(requeue)
{
}

bool Session::answer(bool ok, SeededRandom &rnd)
{
    const Item item = current();
    if (item.retry) {
        ++m_secondTries;
        if (ok)
            ++m_fixed;
        return false;
    }
    ++m_answered;
    if (ok)
        ++m_first;
    else
        m_missed.push_back(item);
    if (ok || !m_requeue)
        return false;
    Item again = item;
    again.retry = true;
    if (again.type == ItemType::Order || again.type == ItemType::Listen)
        again.tiles = shuffled(std::move(again.tiles), rnd);
    m_kept.push_back(std::move(again));
    return true;
}

void Session::again()
{
    if (current().retry)
        return;
    Item item = current();
    item.retry = true;
    m_kept.push_back(std::move(item));
}

void Session::next()
{
    ++m_position;
    if (m_position == m_firstRound) {
        std::move(m_kept.begin(), m_kept.end(), std::back_inserter(m_queue));
        m_kept.clear();
    }
}

// ---- B1 mock exam ----------------------------------------------------------------------------------------

namespace {

bool isWordChar(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

// /\b(voy|vamos|va|van) a \w+|\w+(ré|rás|rá|remos|rán)\b|me gustaría|me encantaría|quiero|espero/, with
// JavaScript's ASCII-only \w and \b, so accented letters count as non-word characters exactly as on the web.
bool mentionsFuture(const std::string &low)
{
    for (const std::string_view phrase : {"me gustaría", "me encantaría", "quiero", "espero"}) {
        if (contains(low, phrase))
            return true;
    }
    const std::size_t n = low.size();
    for (std::size_t p = 0; p < n; ++p) {
        for (const std::string_view verb : {"voy", "vamos", "va", "van"}) {
            const std::size_t after = p + verb.size();
            if (low.compare(p, verb.size(), verb) == 0 && (p == 0 || !isWordChar(low[p - 1]))
                && low.compare(after, 3, " a ") == 0 && after + 3 < n && isWordChar(low[after + 3]))
                return true;
        }
        if (p == 0 || !isWordChar(low[p - 1]))
            continue;
        for (const std::string_view ending : {"ré", "rás", "rá", "remos", "rán"}) {
            if (low.compare(p, ending.size(), ending) != 0)
                continue;
            const std::size_t end = p + ending.size();
            const bool before = isWordChar(low[end - 1]);
            const bool next = end < n && isWordChar(low[end]);
            if (before != next)
                return true;
        }
    }
    return false;
}

} // namespace

std::vector<WritingCheck> writingChecks(std::string_view text)
{
    static const std::array<std::string_view, 15> kPastEndings{"é",   "ó",    "aba", "abas", "ábamos", "aban", "ía",   "ías",
                                                               "íamos", "ían", "ió",  "aron", "ieron",  "iste", "amos"};
    static const std::set<std::string, std::less<>> kPastForms{"fui",   "fue",    "fuimos", "fueron", "tuve", "tuvo",
                                                               "hice",  "hizo",   "estuve", "estuvo", "pude", "dije",
                                                               "vine",  "era",    "eran",   "había"};
    static const std::array<std::string_view, 16> kConnectors{
        "pero",      "aunque",  "porque", "por eso", "sin embargo", "además",   "luego",        "después",
        "al final",  "así que", "primero", "cuando", "mientras",    "de repente", "al principio", "por suerte"};

    const std::vector<std::string> list = words(text);
    const int n = static_cast<int>(list.size());
    const std::string low = " " + normalize(text) + " ";
    const int past = static_cast<int>(std::count_if(list.begin(), list.end(), [](const std::string &w) {
        return kPastForms.contains(w)
               || std::any_of(kPastEndings.begin(), kPastEndings.end(), [&](std::string_view e) { return endsWith(w, e); });
    }));
    const int connectors = static_cast<int>(std::count_if(kConnectors.begin(), kConnectors.end(), [&](std::string_view c) {
        return contains(low, " " + std::string(c) + " ");
    }));
    return {
        {n >= 80 && n <= 170, "Length: " + std::to_string(n) + " words (aim for 100–150)"},
        {past >= 3, "Past tenses used (" + std::to_string(past) + " forms found)"},
        {connectors >= 3, "Connectors (" + std::to_string(connectors) + ": pero, aunque, luego…)"},
        {mentionsFuture(low), "A future plan or wish (voy a…, me gustaría…)"},
    };
}

MockResult mockResult(const MockScores &scores)
{
    const auto ratio = [](int part, int whole) { return whole > 0 ? static_cast<double>(part) / whole : 0.0; };
    MockResult r;
    r.reading = ratio(scores.reading.correct + scores.grammar.correct, scores.reading.total + scores.grammar.total);
    r.listening = ratio(scores.listening.correct, scores.listening.total);
    r.writing = ratio(scores.writingPoints, 4 + kWritingSelfChecks);
    r.speaking = ratio(std::accumulate(scores.speaking.begin(), scores.speaking.end(), 0),
                       kSpeakingRubricMax * static_cast<int>(scores.speaking.size()));
    r.written = (r.reading + r.writing) / 2;
    r.oral = (r.listening + r.speaking) / 2;
    r.overall = (r.reading + r.listening + r.writing + r.speaking) / 4;
    r.pass = r.written >= 0.6 && r.oral >= 0.6;
    return r;
}

} // namespace lq
