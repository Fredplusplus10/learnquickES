// Exports the course from the web app (app/js/content) for the native app: the 25 units with their
// vocabulary, grammar, dialogues and exercises, the trainer content, the B1 mock exam and the immersion
// screen's resources, tips, device missions and exchange phrases.
// Regenerate after changing the lessons:
//   node desktop/tools/export_course.js > desktop/content/course.json
// CI fails if the committed file is out of date.
const LQ = require('../../tests/load')();
const C = LQ.content;

// Authored exercises keep the web app's convention: in 'mc' the first option is the correct one.
const exercise = e => ({
  t: e.t,
  q: e.q || '',
  o: e.o || [],
  a: e.a === undefined ? [] : [].concat(e.a),
  hint: e.hint || '',
  why: e.why || '',
  es: e.es || '',
  en: e.en || ''
});

// The web app's missions are written for an iPhone and a Mac. The native app is for anyone on Windows or
// macOS, so its missions name both, and the phone ones Android too. The ids stay the web app's, so a web
// backup keeps its ticks; 'app' (LearnQuick's interface in Spanish) is left out because the native interface
// is English only.
const missions = [
  { id: 'phone', title: 'Phone in Spanish', how: 'iPhone: Settings → General → Language & Region → iPhone Language → Español (México). Android: Settings → System → Languages → add Español and drag it to the top.' },
  { id: 'mac', title: 'Computer in Spanish', how: 'Windows: Settings → Time & language → Language & region → Add a language → Español (México) → "Set as my Windows display language". Mac: System Settings → General → Language & Region → Applications → "+" → pick your browser → Español.' },
  C.missions.find(m => m.id === 'social'),
  C.missions.find(m => m.id === 'youtube'),
  { id: 'siri', title: 'Voice assistant in Spanish', how: 'Siri: Settings → Siri → Language → Español. Google Assistant: Assistant settings → Languages → Español. Ask it the weather in Spanish every morning.' },
  { id: 'keyboard', title: 'Spanish keyboard', how: 'Phone: add an Español keyboard in its keyboard settings. Windows: add the United States-International keyboard, then type \' and a vowel for á, or ~ and n for ñ. Mac: hold a vowel to pick á é í ó ú, or press Option+E then the vowel.' }
];

const course = {
  phases: C.phases.map(p => ({ n: p.n, en: p.en, es: p.es, desc: p.desc })),
  units: C.units.map(u => ({
    id: u.id, n: u.n, phase: u.phase, title: u.title, es: u.es, canDo: u.canDo, trick: u.trick, art: u.art, coach: u.coach,
    vocab: u.vocab.map(v => ({ id: v.id, es: v.es, en: v.en, ex: v.ex || '', g: v.g || '' })),
    grammar: {
      title: u.grammar.title, hack: u.grammar.hack, tip: u.grammar.tip || '',
      tables: u.grammar.tables.map(t => ({ head: t.head, rows: t.rows })),
      examples: u.grammar.examples
    },
    dialogue: u.dialogue,
    ex: u.ex.map(exercise),
    speak: u.speak
  })),
  practice: {
    pastReasons: C.pastReasons,
    past: C.past,
    cognateRules: C.cognateRules,
    falseFriends: C.falseFriends,
    genderRules: C.genderRules,
    genderWords: C.genderWords,
    pronunciation: C.pronunciation,
    describeFrames: C.describeFrames,
    describe: C.describe
  },
  mock: C.mock,
  immersion: {
    resources: C.resources.map(r => ({ name: r.name, level: r.level, accent: r.accent, kind: r.kind, url: r.url, note: r.note })),
    series: C.series.map(x => ({ name: x.name, level: x.level, years: x.years, kind: x.kind, note: x.note,
      where: x.where.map(([service, detail]) => ({ service, detail })) })),
    seriesWhere: C.seriesWhere,
    seriesTips: C.seriesTips,
    missions: missions.map(m => ({ id: m.id, title: m.title, how: m.how })),
    exchangePhrases: C.exchangePhrases
  }
};
process.stdout.write(JSON.stringify(course, null, 1) + '\n');
