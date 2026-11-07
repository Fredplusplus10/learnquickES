// Golden-master data from the web app's engine (app/js/core). The C++ engine must reproduce it exactly
// (desktop/tests/web_parity_tests.cpp, backup_tests.cpp). Regenerate after changing either side:
//   node desktop/tools/web_reference.js > desktop/tests/data/web_reference.json
// CI fails if the committed file is out of date.
const fs = require('fs');
const path = require('path');
const LQ = require('../../tests/load')();
const { util: U, calendar: C, progress: P, srs: R, verbs: V } = LQ;

const out = { holidays: {}, schedules: {}, demo: {}, answers: [], normalize: {}, verbs: {}, srs: {}, writing: [] };
for (let y = 2020; y <= 2040; y++) out.holidays[y] = [...C.usHolidays(y)].sort();
for (const days of [30, 60, 90]) {
  out.schedules[days] = C.buildSchedule(days).map(e => ({
    n: e.n, phase: e.phase, type: e.type, unit: e.unit ?? -1, practice: e.idx ?? -1, tasks: C.dayTasks(e, days)
  }));
}
// Dates around weekends, holidays, year ends and DST changes.
for (const today of ['2026-03-09', '2026-11-02', '2026-11-27', '2027-01-04', '2027-07-06', '2027-12-27']) {
  U.clock = () => new Date(U.parse(today).getTime() + 12 * 3600 * 1000);
  const S = LQ.store.demoState(today);
  const streak = P.streakInfo(S, today), pace = P.pace(S, today);
  const xp = {}, activity = {};
  for (const [d, a] of Object.entries(S.activity).sort()) {
    xp[d] = a.xp;
    activity[d] = { sec: a.sec, input: a.input, island: a.island, speak: a.speak, ex: a.ex };
  }
  out.demo[today] = {
    start: S.plan.start,
    done: S.done.map(d => d.date),
    units: Object.keys(S.units).map(id => +id.slice(1) - 1).sort((a, b) => a - b),
    checkpoints: Object.keys(S.checkpoints).map(Number).sort(),
    xp,
    activity,
    // In the order they joined the deck, which breaks ties in the review queue.
    cards: Object.entries(S.cards).map(([id, c]) => ({ id, unit: +c.u.slice(1) - 1, box: c.box, due: c.due, added: c.added, reps: c.reps })),
    queue: R.queue(S.cards, today, 20),
    freezes: S.streak.freezes,
    frozen: [...S.streak.frozen].sort(),
    streak: streak.count,
    longest: streak.longest,
    week: streak.week.map(w => w.state),
    pace: { expected: pace.expected, diff: pace.diff, remaining: pace.remaining, target: pace.target }
  };
}

// Answer checking: case, punctuation, spacing and accents.
const cases = [
  ['Estoy', ['estoy']], ['esta', ['está']], ['  ¿Dónde   está? ', ['dónde está']], ['nino', ['niño']], ['', ['hola']],
  ['   ', ['hola']], ['ESTÁN', ['están']], ['somos', ['son', 'somos']], ['la mesa.', ['la mesa']], ['«Hola»', ['hola']],
  ['pinguino', ['pingüino']], ['Él', ['el']], ['tengo  que', ['tengo que']], ['x', ['y']], ['voy a ir', ['Voy a ir.']],
  ['¡Qué bien!', ['que bien']], ['Mañana', ['manana']], ['soy de Chicago', ['Soy de Chicago.']], ['ESTÁ', ['esta']],
  ['él/ella', ['él/ella']], ['(sí)', ['si']], ['café con leche', ['café con leche']], ['dije;', ['dije']],
  ['he aprendido', ['he']], ['ÁÉÍÓÚÜÑ', ['áéíóúüñ']], ['aeiouun', ['ÁÉÍÓÚÜÑ']], ['“citas”', ['citas']]
];
for (const [input, answers] of cases) out.answers.push({ input, answers, result: U.check(input, answers) });
for (const s of ['¿Qué HORA es?', '  a,b;c:d  ', 'Él dijo: «sí».', 'Árbol\tgrande\n', '']) out.normalize[s] = U.normalize(s);

// Every conjugation the verb gym and checkpoints can ask for.
for (const inf of Object.keys(V.VERBS)) {
  out.verbs[inf] = { participle: V.participle(inf) };
  for (const tense of Object.keys(V.TENSES)) out.verbs[inf][tense] = V.conjugate(inf, tense);
}
out.verbCore = V.CORE;

// Spaced repetition: one card through a sequence of grades, each review on the day it falls due.
{
  let card = R.create('2026-11-02'), day = '2026-11-02';
  const steps = [];
  for (const g of [2, 2, 3, 0, 2, 3, 3, 3, 3, 2]) {
    day = card.due;
    card = R.grade(card, g, day);
    steps.push({ grade: g, day, box: card.box, due: card.due, reps: card.reps, lapses: card.lapses });
  }
  out.srs.steps = steps;
  // Ties on the due date go to the weaker box, then to the order the cards joined the deck.
  const deck = {};
  const add = (id, box, due) => { deck[id] = { ...R.create('2026-10-01'), box, due }; };
  add('a', 3, '2026-11-01'); add('b', 1, '2026-11-02'); add('c', 2, '2026-10-30'); add('d', 1, '2026-11-01');
  add('e', 5, '2026-11-03'); add('f', 3, '2026-11-01'); add('g', 2, '2026-10-30'); add('h', 1, '2026-11-02');
  out.srs.deck = Object.entries(deck).map(([id, c]) => ({ id, box: c.box, due: c.due }));
  out.srs.queue = R.queue(deck, '2026-11-02', 20);
  out.srs.queue3 = R.queue(deck, '2026-11-02', 3);
}

// The B1 mock's writing checks live in a view file; evaluate the web app's own function.
{
  const src = fs.readFileSync(path.join(__dirname, '..', '..', 'app', 'js', 'views', 'checkpoint.js'), 'utf8');
  const connectors = /const CONNECTORS = \[[^\]]*\];/.exec(src)[0];
  const fn = /function writingChecks\(text\) \{[\s\S]*?\n  \}\n/.exec(src)[0];
  const writingChecks = new Function('U', 'L', `${connectors}\n${fn}\nreturn writingChecks;`)(U, en => en);
  const texts = [
    '',
    'Hola, Marta: Ayer fui a la playa. Era muy bonita, pero hacía frío. Luego comí en un restaurante y después volví a casa. El año que viene voy a viajar a Perú.',
    'Querido Juan: ¿Cómo estás? Te escribo porque el verano pasado viajé a México con mi familia. Al principio estábamos un poco nerviosos, sin embargo todo salió muy bien. Visitamos Guadalajara y Tequila. La ciudad era preciosa y la gente era muy amable. Cuando llegamos al hotel, hacía mucho calor, así que nadamos en la piscina. Además, comimos tacos todos los días. Mientras caminábamos por el centro, de repente empezó a llover. Por suerte, encontramos un café. Me encantaría volver el año que viene. Espero verte pronto. Un abrazo, Cory',
    'Quiero ir. Iremos mañana. Comeré bien. Él va a comer. Tú vas a cantar.',
    'Hablaré con ella y viajarás a Madrid, pero nadie sabe cuándo será.',
    'antes trabajaba aquí aunque luego me mudé porque mi jefe era terrible y por eso cambié de trabajo además primero estudié inglés'
  ];
  out.writing = texts.map(text => ({ text, checks: writingChecks(text).map(([ok, label]) => ({ ok, label })) }));
}
// A backup of the sample learner as the web app exports it, which the desktop app must restore (backup_tests.cpp).
{
  const today = '2026-11-02', uid = U.uid;
  U.clock = () => new Date(U.parse(today).getTime() + 12 * 3600 * 1000);
  let n = 0;
  U.uid = () => 'id' + n++;  // log entries get random ids; fixed ones keep this file stable
  out.backup = { app: 'LearnQuick', exported: '2026-11-02T12:00:00.000Z', state: LQ.store.demoState(today) };
  U.uid = uid;
}
process.stdout.write(JSON.stringify(out, null, 1) + '\n');
