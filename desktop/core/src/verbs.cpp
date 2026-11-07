#include "lq/verbs.h"

#include <algorithm>

namespace lq {

namespace {

// Irregular data exactly as in verbs.js: six space-separated forms, or empty when the rules apply.
struct VerbData {
    std::string_view infinitive;
    std::string_view en;
    bool core = false;
    std::string_view pres = {};
    std::string_view pret = {};
    std::string_view imp = {};
    std::string_view subj = {};
    std::string_view futureStem = {};
    std::string_view participle = {};
};

constexpr std::string_view kIrPresent = "voy vas va vamos vais van";

const std::vector<VerbData> &verbData()
{
    static const std::vector<VerbData> data{
        {"ser", "to be (identity)", true, "soy eres es somos sois son", "fui fuiste fue fuimos fuisteis fueron",
         "era eras era éramos erais eran", "sea seas sea seamos seáis sean", {}, "sido"},
        {"estar", "to be (state, place)", true, "estoy estás está estamos estáis están",
         "estuve estuviste estuvo estuvimos estuvisteis estuvieron", {}, "esté estés esté estemos estéis estén"},
        {"tener", "to have", true, "tengo tienes tiene tenemos tenéis tienen", "tuve tuviste tuvo tuvimos tuvisteis tuvieron",
         {}, {}, "tendr"},
        {"hacer", "to do, make", true, "hago haces hace hacemos hacéis hacen", "hice hiciste hizo hicimos hicisteis hicieron",
         {}, {}, "har", "hecho"},
        {"poder", "to be able to, can", true, "puedo puedes puede podemos podéis pueden",
         "pude pudiste pudo pudimos pudisteis pudieron", {}, "pueda puedas pueda podamos podáis puedan", "podr"},
        {"querer", "to want; to love", true, "quiero quieres quiere queremos queréis quieren",
         "quise quisiste quiso quisimos quisisteis quisieron", {}, "quiera quieras quiera queramos queráis quieran", "querr"},
        {"ir", "to go", true, kIrPresent, "fui fuiste fue fuimos fuisteis fueron", "iba ibas iba íbamos ibais iban",
         "vaya vayas vaya vayamos vayáis vayan"},
        {"decir", "to say, tell", true, "digo dices dice decimos decís dicen", "dije dijiste dijo dijimos dijisteis dijeron",
         {}, {}, "dir", "dicho"},
        {"hablar", "to speak"},
        {"comer", "to eat"},
        {"vivir", "to live"},
        {"trabajar", "to work"},
        {"ver", "to see, watch", false, "veo ves ve vemos veis ven", "vi viste vio vimos visteis vieron",
         "veía veías veía veíamos veíais veían", {}, {}, "visto"},
        {"dar", "to give", false, "doy das da damos dais dan", "di diste dio dimos disteis dieron", {}, "dé des dé demos deis den"},
        {"saber", "to know (facts)", false, "sé sabes sabe sabemos sabéis saben", "supe supiste supo supimos supisteis supieron",
         {}, "sepa sepas sepa sepamos sepáis sepan", "sabr"},
        {"venir", "to come", false, "vengo vienes viene venimos venís vienen", "vine viniste vino vinimos vinisteis vinieron",
         {}, {}, "vendr"},
        {"salir", "to go out, leave", false, "salgo sales sale salimos salís salen", {}, {}, {}, "saldr"},
        {"poner", "to put", false, "pongo pones pone ponemos ponéis ponen", "puse pusiste puso pusimos pusisteis pusieron",
         {}, {}, "pondr", "puesto"},
        {"llegar", "to arrive", false, {}, "llegué llegaste llegó llegamos llegasteis llegaron", {},
         "llegue llegues llegue lleguemos lleguéis lleguen"},
        {"pensar", "to think", false, "pienso piensas piensa pensamos pensáis piensan", {}, {},
         "piense pienses piense pensemos penséis piensen"},
        {"volver", "to return", false, "vuelvo vuelves vuelve volvemos volvéis vuelven", {}, {},
         "vuelva vuelvas vuelva volvamos volváis vuelvan", {}, "vuelto"},
        {"conocer", "to know (people, places)", false, "conozco conoces conoce conocemos conocéis conocen"},
        {"sentir", "to feel", false, "siento sientes siente sentimos sentís sienten",
         "sentí sentiste sintió sentimos sentisteis sintieron", {}, "sienta sientas sienta sintamos sintáis sientan"},
        {"pedir", "to ask for, order", false, "pido pides pide pedimos pedís piden", "pedí pediste pidió pedimos pedisteis pidieron"},
        {"dormir", "to sleep", false, "duermo duermes duerme dormimos dormís duermen",
         "dormí dormiste durmió dormimos dormisteis durmieron", {}, "duerma duermas duerma durmamos durmáis duerman"},
    };
    return data;
}

const VerbData *findData(std::string_view infinitive)
{
    const auto &data = verbData();
    const auto it = std::find_if(data.begin(), data.end(), [&](const VerbData &v) { return v.infinitive == infinitive; });
    return it == data.end() ? nullptr : &*it;
}

Paradigm split(std::string_view forms)
{
    Paradigm out;
    std::size_t person = 0;
    std::size_t start = 0;
    while (person < out.size() && start <= forms.size()) {
        const std::size_t end = std::min(forms.find(' ', start), forms.size());
        out[person++] = std::string(forms.substr(start, end - start));
        start = end + 1;
    }
    return out;
}

using Endings = std::array<std::string_view, 6>;

// Endings by tense and verb class (-ar, -er, -ir).
const Endings &endings(std::string_view tense, std::string_view type)
{
    static const Endings presAr{"o", "as", "a", "amos", "áis", "an"}, presEr{"o", "es", "e", "emos", "éis", "en"},
        presIr{"o", "es", "e", "imos", "ís", "en"};
    static const Endings pretAr{"é", "aste", "ó", "amos", "asteis", "aron"},
        pretErIr{"í", "iste", "ió", "imos", "isteis", "ieron"};
    static const Endings impAr{"aba", "abas", "aba", "ábamos", "abais", "aban"},
        impErIr{"ía", "ías", "ía", "íamos", "íais", "ían"};
    static const Endings subjAr{"e", "es", "e", "emos", "éis", "en"}, subjErIr{"a", "as", "a", "amos", "áis", "an"};
    const bool ar = type == "ar";
    if (tense == "pres")
        return ar ? presAr : type == "er" ? presEr : presIr;
    if (tense == "pret")
        return ar ? pretAr : pretErIr;
    if (tense == "imp")
        return ar ? impAr : impErIr;
    return ar ? subjAr : subjErIr;
}

constexpr Endings kFuture{"é", "ás", "á", "emos", "éis", "án"};
constexpr Endings kConditional{"ía", "ías", "ía", "íamos", "íais", "ían"};
constexpr Endings kHaber{"he", "has", "ha", "hemos", "habéis", "han"};

std::string_view typeOf(std::string_view infinitive)
{
    return infinitive.substr(infinitive.size() - 2);
}

std::string_view stemOf(std::string_view infinitive)
{
    return infinitive.substr(0, infinitive.size() - 2);
}

Paradigm withEndings(std::string_view stem, const Endings &list)
{
    Paradigm out;
    for (std::size_t i = 0; i < out.size(); ++i)
        out[i] = std::string(stem) + std::string(list[i]);
    return out;
}

// Removes the last character (a whole UTF-8 sequence).
std::string dropLast(const std::string &s)
{
    std::size_t end = s.size();
    while (end > 0 && (static_cast<unsigned char>(s[end - 1]) & 0xC0) == 0x80)
        --end;
    return s.substr(0, end > 0 ? end - 1 : 0);
}

} // namespace

const TenseInfo &tenseInfo(Tense tense)
{
    static const std::array<TenseInfo, 8> info{{
        {"pres", "Present", "Presente", "hoy · normalmente", 0},
        {"irA", "Near future (ir a + inf.)", "Futuro perifrástico", "mañana · plan", 3},
        {"pret", "Preterite (fui)", "Pretérito", "ayer · una vez", 10},
        {"imp", "Imperfect (iba)", "Imperfecto", "antes · de niño", 12},
        {"perf", "Present perfect", "Pretérito perfecto", "alguna vez · hoy", 15},
        {"fut", "Future", "Futuro", "el año que viene", 16},
        {"cond", "Conditional", "Condicional", "cortesía · consejo", 17},
        {"subj", "Present subjunctive", "Subjuntivo presente", "Quiero que…", 20},
    }};
    return info[static_cast<std::size_t>(tense)];
}

std::optional<Tense> tenseFromId(std::string_view id)
{
    for (const Tense tense : kTenses) {
        if (tenseInfo(tense).id == id)
            return tense;
    }
    return std::nullopt;
}

const std::vector<Verb> &verbs()
{
    static const std::vector<Verb> list = [] {
        std::vector<Verb> out;
        for (const VerbData &v : verbData())
            out.push_back({v.infinitive, v.en, v.core});
        return out;
    }();
    return list;
}

const Verb *findVerb(std::string_view infinitive)
{
    const auto &list = verbs();
    const auto it = std::find_if(list.begin(), list.end(), [&](const Verb &v) { return v.infinitive == infinitive; });
    return it == list.end() ? nullptr : &*it;
}

std::vector<std::string_view> coreVerbs()
{
    std::vector<std::string_view> out;
    for (const Verb &v : verbs()) {
        if (v.core)
            out.push_back(v.infinitive);
    }
    return out;
}

std::string participle(std::string_view infinitive)
{
    const VerbData *v = findData(infinitive);
    if (v && !v->participle.empty())
        return std::string(v->participle);
    return std::string(stemOf(infinitive)) + (typeOf(infinitive) == "ar" ? "ado" : "ido");
}

Paradigm conjugate(std::string_view infinitive, Tense tense)
{
    static const VerbData regular{};
    const VerbData *found = findData(infinitive);
    const VerbData &v = found ? *found : regular;
    const std::string_view type = typeOf(infinitive);
    const std::string_view stem = stemOf(infinitive);
    const std::string_view futureStem = v.futureStem.empty() ? infinitive : v.futureStem;
    switch (tense) {
    case Tense::Present:
        return v.pres.empty() ? withEndings(stem, endings("pres", type)) : split(v.pres);
    case Tense::Preterite:
        return v.pret.empty() ? withEndings(stem, endings("pret", type)) : split(v.pret);
    case Tense::Imperfect:
        return v.imp.empty() ? withEndings(stem, endings("imp", type)) : split(v.imp);
    case Tense::Future:
        return withEndings(futureStem, kFuture);
    case Tense::Conditional:
        return withEndings(futureStem, kConditional);
    case Tense::NearFuture: {
        Paradigm out = split(kIrPresent);
        for (std::string &form : out)
            form += " a " + std::string(infinitive);
        return out;
    }
    case Tense::Perfect: {
        Paradigm out;
        const std::string part = participle(infinitive);
        for (std::size_t i = 0; i < out.size(); ++i)
            out[i] = std::string(kHaber[i]) + " " + part;
        return out;
    }
    case Tense::Subjunctive: {
        if (!v.subj.empty())
            return split(v.subj);
        // The yo form of the present minus its -o.
        return withEndings(dropLast(conjugate(infinitive, Tense::Present)[0]), endings("subj", type));
    }
    }
    return {};
}

const std::vector<std::string_view> &pronouns(int person)
{
    static const std::array<std::vector<std::string_view>, 6> list{{
        {"yo"},
        {"tú"},
        {"él", "ella", "usted"},
        {"nosotros", "nosotras"},
        {"vosotros", "vosotras"},
        {"ellos", "ellas", "ustedes"},
    }};
    return list[static_cast<std::size_t>(person)];
}

} // namespace lq
