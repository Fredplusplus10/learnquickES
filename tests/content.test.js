const test = require('node:test');
const assert = require('node:assert/strict');
const LQ = require('./load')();
const { util: U, content: C, verbs: VB, ex: EX } = LQ;

test('25 units, 5 per phase, with complete sections', () => {
  assert.equal(C.units.length, 25);
  for (const ph of [1, 2, 3, 4, 5]) assert.equal(C.units.filter(u => u.phase === ph).length, 5);
  for (const u of C.units) {
    const where = `${u.id} ${u.title}`;
    assert.ok(u.title && u.es && u.canDo && u.coach, where);
    assert.ok([1, 2, 3, 4, 5].includes(u.trick), where + ' trick');
    assert.equal(u.vocab.length, 12, where + ' vocab');
    assert.ok(u.ex.length >= 9, where + ' exercises');
    assert.equal(u.speak.length, 3, where + ' speak');
    assert.ok(u.dialogue.length >= 6 && u.dialogue.every(l => l.length === 3 && l.every(Boolean)), where + ' dialogue');
    assert.ok(u.dialogue.some(l => l[0] === 'Tú'), where + ' learner speaks in the dialogue');
    const g = u.grammar;
    assert.ok(g.title && g.hack && g.tip && g.examples.length >= 3, where + ' grammar');
    for (const t of g.tables) for (const r of t.rows) assert.equal(r.length, t.head.length, `${where} table row ${r[0]}`);
    for (const v of u.vocab) assert.ok(v.id && v.es && v.en && v.ex, `${where} vocab ${v.es}`);
  }
});

test('vocabulary ids are unique across the course', () => {
  const seen = {};
  for (const u of C.units) for (const v of u.vocab) { assert.ok(!seen[v.id], `${v.id} appears in ${seen[v.id]} and ${u.id}`); seen[v.id] = u.id; }
  assert.equal(Object.keys(C.vocabIndex).length, 300);
});

test('authored exercises are well-formed', () => {
  for (const u of C.units) for (const e of u.ex) {
    const where = `${u.id}: ${e.q || e.es}`;
    if (e.t === 'mc') {
      assert.ok(e.o.length >= 2 && new Set(e.o).size === e.o.length, where + ' options');
      const blanks = (e.q.match(/___/g) || []).length;
      if (blanks > 1) assert.equal(e.o[0].split(' … ').length, blanks, where + ' multi-blank answer');
    } else if (e.t === 'fill') {
      assert.equal((e.q.match(/___/g) || []).length, 1, where + ' exactly one blank');
      assert.ok([].concat(e.a).every(a => a && a === a.trim()), where + ' answer');
    } else {
      assert.ok(['order', 'listen'].includes(e.t), where + ' type');
      assert.ok(e.es && e.en && U.words(e.es).length >= 3, where + ' sentence');
    }
  }
});

test('built lesson sessions are solvable', () => {
  for (const u of C.units) {
    const items = EX.fromUnit(u);
    assert.ok(items.length >= 12, u.id);
    for (const it of items) {
      if (it.t === 'mc') assert.ok(it.a >= 0 && it.a < it.o.length, u.id + ' mc answer index');
      if (it.t === 'order' || it.t === 'listen') {
        // The correct tiles exist in the bank and joining them passes the checker.
        const words = it.es.replace(/[¿?¡!.,;:]/g, ' ').split(/\s+/).filter(Boolean);
        const bank = it.tiles.map(t => t.toLowerCase());
        for (const w of words) assert.ok(bank.includes(w.toLowerCase()), `${u.id} tile ${w}`);
        assert.ok(U.check(words.join(' '), [it.es]).ok, `${u.id} ${it.es}`);
      }
      if (it.t === 'gender') assert.ok(it.opts.includes(it.a), u.id + ' gender');
      if (it.t === 'match') assert.equal(new Set(it.pairs.map(p => p[0])).size, it.pairs.length, u.id + ' match pairs unique');
    }
  }
});

test('trainer builders produce valid items', () => {
  for (const it of EX.conj(Object.keys(VB.VERBS), Object.keys(VB.TENSES), 200, true)) assert.ok(it.a[0] && it.pronoun, it.verb);
  for (const it of EX.past(C.past.length)) assert.equal(it.o.length, 2);
  for (const it of EX.cognates(40)) assert.ok(it.a[0] && it.rule);
  for (const it of EX.gender(60)) assert.ok(it.opts.includes(it.a), it.word);
  assert.equal(EX.falseFriends(8).length, 8);
  // Cognates and false friends count towards skills of their own, as in the desktop app.
  for (const it of EX.cognates(10)) assert.equal(it.skill, 'cognates');
  for (const it of EX.falseFriends(8)) assert.equal(it.skill, 'falsefriends');
  assert.equal(EX.describe(5).length, 5);
  assert.equal(EX.mixed([], 10).length, 10);
  assert.ok(EX.sentenceOf(EX.mc('El avión es ___ rápido ___ el tren.', ['más … que', 'tan … que'])).includes('más rápido que'));
});

test('practice content is consistent', () => {
  for (const [q, ok, bad, why] of C.past) { assert.ok(q.includes('___') && ok !== bad, q); assert.ok(C.pastReasons[why], why); }
  for (const [rule, , list] of C.cognateRules) assert.ok(list.length >= 3, rule);
  assert.equal(C.falseFriends.length, 20);
  for (const [es] of C.genderWords) assert.ok(U.gender(es), es);
  assert.equal(C.placement.length, 15);
  assert.deepEqual([0, 6, 7, 11, 12, 15].map(s => C.placementLevel(s).plan), [90, 90, 60, 60, 30, 30]);
  assert.equal(C.mock.reading.questions.length, 5);
  assert.equal(C.mock.listening.questions.length, 5);
  assert.equal(C.mock.speaking.length, 4);
  for (const r of C.resources) assert.ok(/^https:\/\//.test(r.url), r.name);
  for (const s of C.series) assert.ok(['A1', 'A2', 'B1'].includes(s.level) && s.where.length && s.note, s.name);
  assert.deepEqual([...new Set(C.series.map(s => s.level))].sort(), ['A1', 'A2', 'B1'], 'series for every level');
  for (const t of C.tricks) assert.ok([1, 2, 3, 4, 5].includes(t.n) && t.route);
  assert.equal(new Set(C.tricks.map(t => t.n)).size, 5, 'all five tricks are featured');
});
