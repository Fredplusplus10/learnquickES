const test = require('node:test');
const assert = require('node:assert/strict');
const LQ = require('./load')();
const { util: U, calendar: CAL, srs: R, streak: K, verbs: VB, progress: P } = LQ;

test('dates are local calendar days and survive DST changes', () => {
  assert.equal(U.addDays('2026-11-01', 1), '2026-11-02'); // US DST ends Nov 1, 2026
  assert.equal(U.diffDays('2026-03-07', '2026-03-09'), 2); // DST starts Mar 8, 2026
  assert.equal(U.weekStart('2026-11-08'), '2026-11-02'); // Sunday → Monday of that week
});

test('US federal holidays use the observed rule', () => {
  const h2026 = [...CAL.usHolidays(2026)].sort();
  assert.deepEqual(h2026, ['2026-01-01', '2026-01-19', '2026-02-16', '2026-05-25', '2026-06-19', '2026-07-03', '2026-09-07', '2026-10-12', '2026-11-11', '2026-11-26', '2026-12-25']);
  assert.ok(CAL.isHoliday('2021-12-31'), 'New Year 2022 fell on a Saturday → observed Friday Dec 31, 2021');
  assert.ok(!CAL.isHoliday('2026-07-04'));
});

test('study days skip weekends, holidays and days off', () => {
  const plan = { studyDays: [1, 2, 3, 4, 5], skipHolidays: true, daysOff: ['2026-11-04'] };
  assert.ok(CAL.isStudyDay('2026-11-02', plan));
  assert.ok(!CAL.isStudyDay('2026-11-04', plan)); // day off
  assert.ok(!CAL.isStudyDay('2026-11-07', plan)); // Saturday
  assert.ok(!CAL.isStudyDay('2026-11-11', plan)); // Veterans Day
  assert.ok(CAL.isStudyDay('2026-11-11', { ...plan, skipHolidays: false }));
  assert.equal(CAL.nthStudyDay('2026-11-02', 5, plan), '2026-11-09');
  assert.equal(CAL.countStudyDays('2026-11-02', '2026-11-13', plan), 8);
});

for (const days of [30, 60, 90]) {
  test(`${days}-day schedule covers all 25 units in order with 5 checkpoints`, () => {
    const s = CAL.buildSchedule(days);
    assert.equal(s.length, days);
    assert.deepEqual(s.filter(e => e.type === 'lesson').map(e => e.unit), Array.from({ length: 25 }, (_, i) => i));
    const cps = s.filter(e => e.type === 'checkpoint');
    assert.deepEqual(cps.map(e => e.n), [1, 2, 3, 4, 5].map(p => p * days / 5));
    for (const e of s.filter(x => x.type === 'lesson')) assert.equal(e.phase, Math.floor(e.unit / 5) + 1);
    assert.deepEqual(CAL.dayTasks(s[days - 1], days), ['review', 'final']);
    for (const e of s) assert.ok(CAL.dayMinutes(CAL.dayTasks(e, days)) <= 50);
  });
}

test('streak bridges weekends and non-study days, breaks on a missed study day', () => {
  const sched = d => CAL.isStudyDay(d, { studyDays: [1, 2, 3, 4, 5], skipHolidays: true, daysOff: [] });
  const active = new Set(['2026-11-05', '2026-11-06', '2026-11-09']); // Thu, Fri, (weekend), Mon
  assert.equal(K.current('2026-11-09', active, new Set(), sched), 3);
  assert.equal(K.current('2026-11-10', active, new Set(), sched), 3, 'today without activity does not break it');
  assert.equal(K.current('2026-11-12', active, new Set(), sched), 0, 'Tuesday missed → broken'); // Wed 12 (Veterans 11 is a holiday)
  const r = K.freezesNeeded('2026-11-12', active, new Set(), sched, 1);
  assert.deepEqual(r.used, ['2026-11-10']);
  assert.equal(K.current('2026-11-12', active, new Set(r.used), sched), 3, 'a freeze keeps it alive');
  assert.equal(K.earned(5, true, 0), 1);
  assert.equal(K.earned(3, false, 1), 1, 'studying on a day off earns a freeze');
  assert.equal(K.earned(10, false, 2), 0, 'capped at 2');
});

test('spaced repetition intervals expand and reset', () => {
  let c = R.create('2026-11-02');
  assert.equal(c.due, '2026-11-03');
  c = R.grade(c, 2, '2026-11-03'); assert.equal(c.box, 2); assert.equal(c.due, '2026-11-05');
  c = R.grade(c, 3, '2026-11-05'); assert.equal(c.box, 4); assert.equal(c.due, '2026-11-13');
  assert.equal(R.status(c), 'remembered');
  c = R.grade(c, 0, '2026-11-13'); assert.equal(c.box, 1); assert.equal(c.lapses, 1);
  const q = R.queue({ a: { due: '2026-11-01', box: 3 }, b: { due: '2026-10-30', box: 2 }, c: { due: '2026-12-01', box: 1 } }, '2026-11-02');
  assert.deepEqual(q, ['b', 'a']);
});

test('answer checking ignores case/punctuation and flags accent-only slips', () => {
  assert.deepEqual(U.check('  ¿Dónde está? ', ['dónde está']).ok, true);
  const r = U.check('estas', ['estás']); assert.ok(r.ok && r.almost);
  assert.ok(!U.check('esta', ['estoy']).ok);
  assert.ok(U.check('', ['x']).empty);
  assert.equal(U.gender('el mapa'), 'm'); assert.equal(U.gender('la mano'), 'f'); assert.equal(U.gender('cansado / cansada'), null);
});

test('conjugator produces the expected forms', () => {
  const cases = [['ser', 'pret', 2, 'fue'], ['ir', 'pret', 0, 'fui'], ['tener', 'fut', 0, 'tendré'], ['hacer', 'perf', 0, 'he hecho'], ['ir', 'subj', 3, 'vayamos'],
    ['hablar', 'cond', 2, 'hablaría'], ['comer', 'imp', 3, 'comíamos'], ['vivir', 'pres', 3, 'vivimos'], ['conocer', 'subj', 0, 'conozca'], ['pedir', 'subj', 3, 'pidamos'],
    ['decir', 'cond', 0, 'diría'], ['poner', 'perf', 5, 'han puesto'], ['ver', 'imp', 0, 'veía'], ['salir', 'fut', 5, 'saldrán'], ['ir', 'irA', 0, 'voy a ir'], ['llegar', 'pret', 0, 'llegué'],
    ['dormir', 'pret', 2, 'durmió'], ['estar', 'subj', 1, 'estés'], ['trabajar', 'subj', 0, 'trabaje'], ['venir', 'subj', 0, 'venga']];
  for (const [v, t, p, want] of cases) assert.equal(VB.form(v, t, p), want, `${v} ${t} ${p}`);
  for (const v of Object.keys(VB.VERBS)) for (const t of Object.keys(VB.TENSES)) {
    const forms = VB.conjugate(v, t);
    assert.equal(forms.length, 6, `${v} ${t}`);
    for (const f of forms) assert.ok(f && !/undefined/.test(f), `${v} ${t}: ${f}`);
  }
});

test('a learner can complete the whole 30-day plan; streak, pace and freezes behave', () => {
  const S = P.defaultState('2026-11-02');
  S.plan.days = 30; S.onboarded = true;
  let d = '2026-11-02', studied = 0, events = [];
  while (!P.planStatus(S).finished) {
    if (CAL.isStudyDay(d, S.plan)) {
      const st = P.planStatus(S);
      events.push(...P.recordAnswer(S, 'verbs', { ok: true }, d));
      if (st.entry.type === 'checkpoint') events.push(...P.saveCheckpoint(S, st.entry.phase, 0.8, d));
      for (const t of st.tasks) if (t === 'lesson') events.push(...P.learnUnit(S, st.unit, 0.9, d)); else if (t !== 'checkpoint' && t !== 'final') events.push(...P.markTask(S, t, d));
      studied++;
    }
    d = U.addDays(d, 1);
    assert.ok(studied <= 30, 'plan should finish in 30 study days');
  }
  assert.equal(S.done.length, 30);
  assert.equal(Object.keys(S.units).length, 25);
  assert.equal(Object.keys(S.checkpoints).length, 5);
  assert.equal(events.filter(e => e.type === 'dayComplete').length, 30);
  assert.equal(events.filter(e => e.type === 'phaseComplete').length, 5);
  assert.equal(events.filter(e => e.type === 'streak').length, 30, 'one streak event per day');
  const last = S.done[29].date, info = P.streakInfo(S, last);
  assert.equal(info.count, 30, 'weekends and Thanksgiving never broke the streak');
  assert.equal(Object.keys(S.cards).length, 300);
  assert.equal(P.pace(S, U.addDays(last, 1)).remaining, 0);
  // Missed study day with a freeze in hand → consumed on next open.
  S.streak.freezes = 1;
  const missed = CAL.nthStudyDay(U.addDays(last, 1), 1, S.plan), back = CAL.nthStudyDay(U.addDays(last, 1), 2, S.plan);
  const ev = P.applyFreezes(S, back);
  assert.deepEqual(ev, [{ type: 'freezeUsed', dates: [missed] }]);
  assert.equal(P.streakInfo(S, back).count, 30);
  const ach = P.checkAchievements(S, back).map(e => e.achievement.id);
  for (const id of ['first_lesson', 'streak_5', 'streak_15', 'streak_30', 'phase_1', 'phase_3', 'b1_mock']) assert.ok(ach.includes(id), id);
});

test('pace compares completed days with elapsed study days', () => {
  const S = P.defaultState('2026-11-02');
  S.plan.start = '2026-11-02';
  const p = P.pace(S, '2026-11-09'); // 5 study days elapsed (Mon–Fri), none done
  assert.equal(p.expected, 5); assert.equal(p.diff, -5); assert.equal(p.remaining, 60);
  assert.equal(p.target, CAL.nthStudyDay('2026-11-09', 60, S.plan));
});

test('migrate keeps old backups loadable', () => {
  const S = P.migrate({ profile: { name: 'Sam' }, plan: { days: '90' }, activity: {} }, '2026-11-02');
  assert.equal(S.profile.name, 'Sam'); assert.equal(S.profile.accent, 'es-MX'); assert.equal(S.plan.days, 90); assert.deepEqual(S.cards, {});
  assert.equal(P.migrate({ plan: { days: 45 } }, '2026-11-02').plan.days, 60);
});

test('the series level starts at the placement and rises with the phase 2 and 4 checkpoints', () => {
  const S = P.migrate({ profile: { level: 'A1' } }, '2026-11-02');
  assert.equal(P.seriesLevel(S), 'A1');
  S.checkpoints[2] = { date: '2026-11-20', score: 0.7, best: 0.7 }; assert.equal(P.seriesLevel(S), 'A2');
  S.checkpoints[4] = { date: '2026-12-20', score: 0.9, best: 0.9 }; assert.equal(P.seriesLevel(S), 'B1');
  assert.equal(P.seriesLevel(P.migrate({ profile: { level: 'A2+ / B1−' } }, '2026-11-02')), 'A2');
});

test('a tampered backup cannot put markup on the page', () => {
  const bad = '<img src=x onerror=alert(1)>';
  const S = P.migrate({
    profile: { name: bad, level: bad, accent: bad, rate: bad },
    plan: { days: 30, start: bad, studyDays: [1, bad], daysOff: [bad, '2026-11-11'] },
    streak: { freezes: bad, frozen: [bad], longest: bad },
    activity: { '2026-11-02': { xp: bad, input: '25', ex: { verbs: [bad, 3], cognates: bad } }, [bad]: { xp: 5 } },
    immersion: [{ id: bad, date: '2026-11-02', min: bad, kind: bad }, { id: 'a', date: '2026-11-02', min: '30', kind: bad, source: bad, imported: true }],
    exchanges: [{ date: bad, min: 20 }], missions: { phone: bad, app: true }, achievements: { first: bad },
    units: { u1: { date: '2026-11-02', score: bad, best: 0.8, attempts: bad } }, cards: { hola: bad, adios: { box: bad, due: bad } }
  }, '2026-11-02');
  const json = JSON.stringify(S);
  // No angle bracket or quote outside the free-text fields, which every view escapes.
  assert.ok(!/[<>]/.test(json.replace(/"(name|source)":"[^"]*"/g, '')), 'markup only survives in escaped text fields');
  assert.equal(S.profile.name, bad.slice(0, 30)); // text the views always escape
  assert.equal(S.plan.start, '2026-11-02'); assert.deepEqual(S.plan.daysOff, ['2026-11-11']);
  assert.equal(S.streak.freezes, 0); assert.deepEqual(S.streak.frozen, []);
  assert.deepEqual(Object.keys(S.activity), ['2026-11-02']);
  assert.deepEqual(S.activity['2026-11-02'].ex, { verbs: [0, 3] }); assert.equal(S.activity['2026-11-02'].input, 25);
  assert.equal(S.immersion.length, 1); assert.equal(S.immersion[0].min, 30); assert.equal(S.immersion[0].imported, true);
  assert.deepEqual(S.exchanges, []); assert.deepEqual(S.missions, { app: true }); assert.deepEqual(S.achievements, {});
  assert.deepEqual(Object.keys(S.cards), ['adios']); assert.equal(S.cards.adios.box, 1);
});

test('demo state is internally consistent', () => {
  U.clock = () => new Date(2026, 10, 2, 12);
  const S = LQ.store.demoState('2026-11-02');
  U.clock = () => new Date();
  assert.ok(S.onboarded && S.done.length > 5);
  assert.ok(Object.keys(S.units).length >= 3);
  const info = P.streakInfo(S, '2026-11-02');
  assert.ok(info.count >= 5, 'demo streak survives thanks to the freeze, got ' + info.count);
});

test('a backup up to 16 MB is read, and a bigger file is refused before parsing', () => {
  const S = LQ.store.demoState('2026-11-02');
  const text = JSON.stringify({ app: 'LearnQuick', state: S }, null, 1);
  const padded = text + ' '.repeat(LQ.store.MAX_BACKUP_BYTES - text.length);
  assert.equal(padded.length, LQ.store.MAX_BACKUP_BYTES);
  assert.equal(LQ.store.parseBackup(padded).profile.name, 'Cory');
  assert.throws(() => LQ.store.parseBackup(padded + ' '), /too large/);
});
