/* Exercise engine: builders turn content into normalized items; run() plays any list of items in a container
   (lessons, trainers, spaced repetition, checkpoints) and reports results. Wrong answers come back once. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, R = LQ.srs, C = LQ.content, V = LQ.verbs;
  const { ico, esc, speakBtn, wordHTML } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const PRAISE = ['¡Muy bien!', '¡Perfecto!', '¡Excelente!', '¡Eso es!', '¡Correcto!'];
  const FILLER = ['es', 'está', 'muy', 'tengo', 'el', 'la', 'un', 'de', 'que', 'pero', 'con', 'mi', 'su', 'por', 'hay', 'son'];

  // ---- Builders ------------------------------------------------------------------------------------------
  function mc(q, options, extra = {}) { const order = U.shuffle(options.map((_, i) => i)); return { t: 'mc', q, o: order.map(i => options[i]), a: order.indexOf(0), ...extra }; }
  function tiles(es, pool) {
    const words = es.replace(/[¿?¡!.,;:]/g, ' ').split(/\s+/).filter(Boolean);
    words[0] = words[0].toLowerCase(); // a capital letter would give the first tile away
    const lower = new Set(words.map(w => w.toLowerCase()));
    return U.shuffle(words.concat(U.sample([...new Set(pool)].filter(w => !lower.has(w.toLowerCase())), 2)));
  }
  const vocabPool = unit => (unit ? unit.vocab : []).map(v => v.es.replace(/^(el|la|los|las)\s+/i, '')).filter(w => !/[\s/+…¿?¡!]/.test(w)).concat(FILLER);
  function fromRaw(raw, unit, extra = {}) {
    const base = { skill: raw.t === 'listen' ? 'listening' : 'grammar', ...extra };
    if (raw.t === 'mc') return mc(raw.q, raw.o, { ...base, why: raw.why });
    if (raw.t === 'fill') return { t: 'fill', q: raw.q, a: [].concat(raw.a), hint: raw.hint, why: raw.why, ...base };
    return { t: raw.t, es: raw.es, en: raw.en, tiles: tiles(raw.es, vocabPool(unit)), ...base };
  }
  function genderItem(v, note) {
    const m = /^(el|la|los|las)\s+(.*)$/i.exec(v.es), art = m[1].toLowerCase(), plural = art.length > 2;
    return { t: 'gender', word: m[2], a: art, opts: plural ? ['los', 'las'] : ['el', 'la'], en: v.en, full: v.es, note, skill: 'gender' };
  }
  function vocabItem(v, list) {
    const others = U.sample(list.filter(x => x.en !== v.en), 2).map(x => x.en);
    return mc(`¿Qué significa «${v.es}»?`, [v.en, ...others], { skill: 'vocab', audio: v.es, why: v.ex ? `Ejemplo: ${v.ex}` : '' });
  }
  function fromUnit(unit) {
    const authored = unit.ex.map(e => fromRaw(e, unit));
    const nouns = unit.vocab.filter(v => v.g && !v.es.includes('/'));
    const match = { t: 'match', pairs: U.sample(unit.vocab.filter(v => v.es.length < 26), 5).map(v => [v.es, v.en]), skill: 'vocab' };
    const gender = U.sample(nouns, Math.min(2, nouns.length)).map(v => genderItem(v));
    return [match, ...U.sample(unit.vocab, 2).map(v => vocabItem(v, unit.vocab)), ...authored.slice(0, 4), ...gender, ...authored.slice(4)];
  }
  // Mixed review across completed units (practice days).
  function mixed(units, n = 10) {
    const pool = units.length ? units : [C.units[0]];
    const items = [];
    for (const u of pool) for (const e of u.ex) items.push({ e, u });
    const picked = U.sample(items, n - 2).map(({ e, u }) => fromRaw(e, u));
    const words = pool.flatMap(u => u.vocab);
    return U.shuffle([...picked, ...U.sample(words, 2).map(v => vocabItem(v, words))]);
  }
  function conj(verbs, tenses, n = 10, vosotros = false) {
    const persons = [0, 1, 2, 3, 5].concat(vosotros ? [4] : []);
    return Array.from({ length: n }, () => {
      const verb = U.sample(verbs, 1)[0], tense = U.sample(tenses, 1)[0], person = U.sample(persons, 1)[0];
      return { t: 'conj', verb, tense, person, pronoun: U.sample(V.PERSONS[person], 1)[0], a: [V.form(verb, tense, person)], skill: 'verbs' };
    });
  }
  const past = (n = 10) => U.sample(C.past, n).map(([q, ok, bad, why]) => mc(q, [ok, bad], { skill: 'past', why: C.pastReasons[why] }));
  function cognates(n = 10) {
    const all = C.cognateRules.flatMap(([en, es, list]) => list.map(([a, b]) => ({ rule: `${en} → ${es}`, en: a, es: b })));
    return U.sample(all, n).map(c => ({ t: 'type', prompt: L('Turn this English word into Spanish:', 'Convierte esta palabra inglesa en español:'), q: c.en, a: [c.es], why: c.rule, rule: c.rule, skill: 'cognates' }));
  }
  const falseFriends = (n = 8) => U.sample(C.falseFriends, n).map(([es, ok, bad, real]) => mc(`«${es}» means…`, [ok, bad], { skill: 'falsefriends', audio: es, why: L(`False friend! "${bad}" in Spanish is «${real}».`, `¡Falso amigo! "${bad}" en español es «${real}».`) }));
  function gender(n = 12) {
    const course = Object.values(C.vocabIndex).filter(v => v.g && !v.es.includes('/')).map(v => [v.es, v.en, '']);
    return U.sample(C.genderWords.concat(U.sample(course, 15)), n).map(([es, en, note]) => genderItem({ es, en }, note));
  }
  const cards = ids => ids.map(id => ({ t: 'card', id, v: C.vocabIndex[id], skill: 'review' })).filter(c => c.v);
  const speakItems = phrases => phrases.map(p => ({ t: 'speak', es: p, skill: 'speaking' }));
  const describe = (n = 5) => U.sample(C.describe, n).map(([en, es, model]) => ({ t: 'describe', en, es, model, skill: 'describe' }));

  // ---- Rendering ---------------------------------------------------------------------------------------------
  const KIND = {
    mc: () => L('Choose the right answer', 'Elige la respuesta correcta'), fill: () => L('Fill in the blank', 'Completa la frase'),
    order: () => L('Build the sentence in Spanish', 'Construye la frase en español'), listen: () => L('Listen and build what you hear', 'Escucha y construye lo que oyes'),
    match: () => L('Match the pairs', 'Une las parejas'), gender: () => L('El or la? Learn the article with the noun', '¿El o la? Aprende el artículo con el sustantivo'),
    type: () => L('Type the answer', 'Escribe la respuesta'), conj: () => L('Conjugate the verb', 'Conjuga el verbo'),
    card: () => L('Review', 'Repaso'), speak: () => L('Say it out loud', 'Dilo en voz alta'), describe: () => L('Describe it in Spanish — no English!', 'Descríbelo en español, ¡sin inglés!')
  };
  const blankInput = (item, val = '') => `<input id="exInput" class="blank" value="${esc(val)}" autocomplete="off" autocapitalize="off" autocorrect="off" spellcheck="false" lang="es" aria-label="${L('Your answer', 'Tu respuesta')}" style="width:${Math.max(5, ...item.a.map(a => a.length)) + 2}ch">`;
  // Full Spanish sentence for the "listen again" button after checking.
  function sentenceOf(item) {
    if (item.t === 'fill') return item.q.replace('___', item.a[0]);
    if (item.t === 'mc' && item.q.includes('___')) return item.o[item.a].split(' … ').reduce((s, part) => s.replace('___', part), item.q).replace(/\(.*?\)/g, '').trim();
    if (item.t === 'mc' && item.audio) return item.audio;
    if (item.t === 'order' || item.t === 'listen') return item.es;
    if (item.t === 'conj') return `${item.pronoun} ${item.a[0]}`;
    if (item.t === 'gender') return item.full;
    if (item.t === 'type') return item.a[0];
    return '';
  }

  function body(cur) {
    const it = cur.item, done = cur.checked;
    switch (it.t) {
      case 'mc': return `<p class="ex-q">${esc(it.q)} ${it.audio ? speakBtn(it.audio) : ''}</p><div class="ex-options" role="radiogroup">${it.o.map((o, i) => {
        const state = done ? (i === it.a ? 'correct' : i === cur.sel ? 'incorrect' : '') : (i === cur.sel ? 'selected' : '');
        return `<button type="button" class="ex-option ${state}" role="radio" aria-checked="${i === cur.sel}" data-ex="opt" data-i="${i}" ${done ? 'disabled' : ''}><kbd>${i + 1}</kbd><span>${esc(o)}</span></button>`;
      }).join('')}</div>`;
      case 'fill': { const [a, b] = it.q.split('___'); return `<p class="ex-q ex-fill" lang="es">${esc(a)}${blankInput(it, cur.input)}${esc(b || '')}</p>${it.hint ? `<p class="ex-hint">(${esc(it.hint)})</p>` : ''}${done ? '' : LQ.ui.accentBar('exInput')}`; }
      case 'type': return `<p class="ex-hint">${esc(it.prompt)}</p><p class="ex-big">${esc(it.q)}</p><input id="exInput" class="text-input" value="${esc(cur.input || '')}" lang="es" autocomplete="off" autocapitalize="off" spellcheck="false" aria-label="${L('Spanish word', 'Palabra en español')}" ${done ? 'disabled' : ''}>${done ? '' : LQ.ui.accentBar('exInput')}`;
      case 'conj': { const t = V.TENSES[it.tense]; return `<div class="conj-head"><p class="ex-big" lang="es">${esc(it.verb)}</p><span class="tag">${esc(L(t.en, t.es))}</span><small>${esc(V.VERBS[it.verb].en)} · ${esc(t.cue)}</small></div><p class="ex-q ex-fill" lang="es"><b>${esc(it.pronoun)}</b> ${blankInput(it, cur.input)}</p>${done ? '' : LQ.ui.accentBar('exInput')}`; }
      case 'order': case 'listen': {
        const bank = it.tiles.map((w, i) => ({ w, i })).filter(x => !cur.picked.includes(x.i));
        const head = it.t === 'order' ? `<p class="ex-q">${esc(it.en)}</p>` : `<div class="listen-head"><button type="button" class="circle-play" data-speak="${esc(it.es)}" aria-label="${L('Play the sentence', 'Reproducir la frase')}">${ico('play', 'white')}</button><button type="button" class="secondary-btn" data-speak="${esc(it.es)}" data-rate="0.65">${ico('turtle')}${L('Slow', 'Lento')}</button>${done ? `<p class="ex-q" lang="es">${esc(it.es)}<br><small>${esc(it.en)}</small></p>` : ''}</div>`;
        return `${head}<div class="tile-answer" aria-label="${L('Your sentence', 'Tu frase')}">${cur.picked.map(i => `<button type="button" class="tile picked" data-ex="unpick" data-i="${i}" ${done ? 'disabled' : ''} lang="es">${esc(it.tiles[i])}</button>`).join('') || `<span class="tile-placeholder">${L('Tap the words in order', 'Toca las palabras en orden')}</span>`}</div><div class="tile-bank">${bank.map(x => `<button type="button" class="tile" data-ex="pick" data-i="${x.i}" ${done ? 'disabled' : ''} lang="es">${esc(x.w)}</button>`).join('')}</div>`;
      }
      case 'gender': return `<p class="ex-big gender-q" lang="es"><span class="blank-art">${done ? esc(it.a) : '___'}</span> ${esc(it.word)}</p><p class="ex-hint">${esc(it.en)}</p><div class="gender-opts">${it.opts.map((o, i) => `<button type="button" class="gender-btn g-${i ? 'f' : 'm'} ${done && o === it.a ? 'correct' : done && o === cur.sel ? 'incorrect' : ''}" data-ex="gender" data-v="${o}" ${done ? 'disabled' : ''}><kbd>${i + 1}</kbd>${o}</button>`).join('')}</div>`;
      case 'match': {
        const left = cur.left || (cur.left = U.shuffle(it.pairs.map((p, i) => i))), right = cur.right || (cur.right = U.shuffle(it.pairs.map((p, i) => i)));
        const cell = (side, i, text) => { const matched = cur.matched.includes(i), sel = cur.pick && cur.pick.side === side && cur.pick.i === i, bad = cur.flash && cur.flash[side] === i; return `<button type="button" class="match-cell ${matched ? 'matched' : ''} ${sel ? 'selected' : ''} ${bad ? 'incorrect' : ''}" data-ex="match" data-side="${side}" data-i="${i}" ${matched ? 'disabled' : ''} ${side === 'l' ? 'lang="es"' : ''}>${esc(text)}</button>`; };
        return `<div class="match-grid"><div>${left.map(i => cell('l', i, it.pairs[i][0])).join('')}</div><div>${right.map(i => cell('r', i, it.pairs[i][1])).join('')}</div></div>`;
      }
      case 'card': {
        const v = it.v, card = LQ.state.cards[it.id], reverse = card && card.box >= 3; // stronger cards are asked English → Spanish
        const front = reverse ? `<p class="ex-hint">${L('How do you say it in Spanish?', '¿Cómo se dice en español?')}</p><p class="ex-big">${esc(v.en)}</p>` : `<p class="ex-big">${wordHTML(v)} ${speakBtn(v.es)}</p>`;
        const back = reverse ? `<p class="ex-big">${wordHTML(v)} ${speakBtn(v.es)}</p>` : `<p class="ex-big ex-en">${esc(v.en)}</p>`;
        const iv = g => { const c = R.grade(card || R.create(U.today()), g, U.today()); return U.diffDays(U.today(), c.due); };
        return `<div class="flashcard ${cur.revealed ? 'revealed' : ''}">${front}${cur.revealed ? `<hr>${back}${v.ex ? `<p class="card-ex" lang="es">${esc(v.ex)} ${speakBtn(v.ex)}</p>` : ''}` : ''}</div>
          ${cur.revealed ? `<div class="grade-row"><button type="button" class="grade-btn again" data-ex="grade" data-g="0"><kbd>1</kbd>${L('Again', 'Otra vez')}<small>${L('tomorrow', 'mañana')}</small></button><button type="button" class="grade-btn good" data-ex="grade" data-g="2"><kbd>2</kbd>${L('Good', 'Bien')}<small>${iv(2)} ${L('days', 'días')}</small></button><button type="button" class="grade-btn easy" data-ex="grade" data-g="3"><kbd>3</kbd>${L('Easy', 'Fácil')}<small>${iv(3)} ${L('days', 'días')}</small></button></div>` : ''}`;
      }
      case 'speak': return LQ.ui.shadow(it.es, it.en);
      case 'describe': return `<p class="ex-hint">${L('Explain this without saying the word — as if you could not remember it:', 'Explícalo sin decir la palabra, como si no la recordaras:')}</p><p class="ex-big">${esc(it.en)}</p>
        <div class="frames">${C.describeFrames.map(f => `<span class="frame" lang="es">${esc(f)}</span>`).join('')}</div>
        <textarea id="exInput" class="text-area" rows="3" lang="es" placeholder="${L('Write or say your description in Spanish…', 'Escribe o di tu descripción en español…')}">${esc(cur.input || '')}</textarea>
        ${cur.revealed ? `<div class="model"><p><b>${L('A model answer:', 'Una respuesta modelo:')}</b> <span lang="es">${esc(it.model)}</span> ${speakBtn(it.model)}</p><p><b>${L('The word:', 'La palabra:')}</b> <span lang="es">${esc(it.es)}</span></p></div>` : ''}`;
    }
    return '';
  }

  function feedback(cur) {
    const it = cur.item, r = cur.result;
    if (!cur.checked || !r) return '';
    const sentence = sentenceOf(it), play = sentence ? speakBtn(sentence, L('Listen to the sentence', 'Escuchar la frase')) : '';
    const why = it.why ? `<p class="why">${esc(it.why)}</p>` : '';
    if (r.ok) return `<div class="fb good"><b>${U.sample(PRAISE, 1)[0]}</b> ${play}${r.almost ? `<p>${L('Watch the accents:', 'Ojo con los acentos:')} <b lang="es">${esc(r.expected)}</b></p>` : ''}${it.t === 'listen' || it.t === 'order' ? `<p lang="es">${esc(it.es)}</p>` : ''}${why}</div>`;
    const expected = it.t === 'mc' ? it.o[it.a] : it.t === 'gender' ? `${it.a} ${it.word}` : it.t === 'match' ? '' : r.expected;
    return `<div class="fb bad"><b>${L('Not quite.', 'Casi.')}</b>${expected ? ` ${L('Correct answer:', 'Respuesta correcta:')} <b lang="es">${esc(expected)}</b>` : ''} ${play}${it.note ? `<p>${esc(it.note)}</p>` : ''}${why}${cur.requeued ? `<p class="again-note">${L('You will see this one again at the end.', 'Volverás a ver esta al final.')}</p>` : ''}</div>`;
  }

  // ---- Session runner ----------------------------------------------------------------------------------------
  function run(el, opts) {
    const S = LQ.state, today = U.today(), queue = opts.items.slice(), requeue = opts.requeue !== false;
    const res = { answered: 0, first: 0, total: queue.length, xp0: P.act(S, today).xp, selfRated: 0, wrong: [] };
    let pos = 0, cur;
    const selfRated = t => ['card', 'speak', 'describe'].includes(t);

    function start() {
      if (pos >= queue.length) return finish();
      cur = { item: queue[pos], sel: null, picked: [], matched: [], checked: false, input: '' };
      draw();
      if (cur.item.t === 'listen') setTimeout(() => LQ.speech.speak(cur.item.es), 250);
      if (cur.item.t === 'card' && cur.item.v && !(S.cards[cur.item.id] && S.cards[cur.item.id].box >= 3)) setTimeout(() => LQ.speech.speak(cur.item.v.es), 200);
    }
    function canCheck() {
      const t = cur.item.t;
      if (t === 'mc') return cur.sel !== null;
      if (t === 'fill' || t === 'type' || t === 'conj') return !!U.normalize(cur.input);
      if (t === 'order' || t === 'listen') return cur.picked.length > 0;
      return false;
    }
    const btn = (ex, label, cls = 'primary-btn', attrs = '') => `<button type="button" class="${cls}" data-ex="${ex}" ${attrs}>${label}</button>`;
    function actions(it) {
      if (cur.checked) return btn('next', `${L('Continue', 'Continuar')} <kbd>↵</kbd>`);
      const skip = btn('skip', L("I don't know", 'No lo sé'), 'secondary-btn left-button');
      switch (it.t) {
        case 'card': return cur.revealed ? '' : btn('reveal', `${L('Show answer', 'Mostrar respuesta')} <kbd>␣</kbd>`);
        case 'describe': return cur.revealed ? btn('self', L('I struggled', 'Me costó'), 'secondary-btn', 'data-ok="0"') + btn('self', L('I managed', 'Lo logré'), 'primary-btn', 'data-ok="1"') : btn('reveal', L('Show a model', 'Ver un modelo'));
        case 'speak': return btn('self', ico('check', 'white') + L('I said it', 'Ya lo dije'), 'primary-btn', 'data-ok="1"');
        case 'match': return '';
        case 'gender': return skip;
        default: return skip + btn('check', `${ico('check', 'white')}${L('Check', 'Comprobar')} <kbd>↵</kbd>`, 'primary-btn', canCheck() ? '' : 'disabled');
      }
    }
    function draw() {
      const it = cur.item, pct = Math.round(pos / queue.length * 100);
      el.innerHTML = `<div class="session">
        <div class="session-top"><div class="session-bar" role="progressbar" aria-valuenow="${pct}" aria-valuemin="0" aria-valuemax="100"><span style="width:${pct}%"></span></div><span class="session-count">${Math.min(pos + 1, queue.length)} / ${queue.length}</span>${opts.onQuit ? btn('quit', L('Exit', 'Salir'), 'text-link') : ''}</div>
        <section class="card ex-card"><p class="ex-kind">${esc(KIND[it.t]())}${it.retry ? ` · <span class="retry-tag">${L('second try', 'segundo intento')}</span>` : ''}</p>${body(cur)}</section>
        <div class="ex-feedback" aria-live="polite">${feedback(cur)}</div>
        <div class="ex-actions">${actions(it)}</div>
      </div>`;
      const input = el.querySelector('#exInput');
      if (input && !cur.checked) { input.focus(); input.setSelectionRange(input.value.length, input.value.length); return; }
      const target = el.querySelector('[data-ex="next"]') || el.querySelector('.ex-option.selected') || el.querySelector('[data-ex="reveal"], .ex-option, .tile, .gender-btn, .match-cell:not([disabled])');
      if (target) target.focus({ preventScroll: true });
    }
    function grade(result) {
      const it = cur.item;
      cur.checked = true; cur.result = result;
      if (!it.retry) { res.answered++; if (result.ok) res.first++; else res.wrong.push(it); }
      if (opts.record !== false) LQ.app.handle(P.recordAnswer(S, it.skill, { ...result, retry: it.retry }, today));
      if (!result.ok && requeue && !it.retry) { queue.push({ ...it, retry: true, ...(it.t === 'order' || it.t === 'listen' ? { tiles: U.shuffle(it.tiles) } : {}) }); cur.requeued = true; }
      LQ.app.save(); draw();
      opts.onAnswer && opts.onAnswer(it, result);
    }
    function check() {
      const it = cur.item;
      if (!canCheck()) return;
      if (it.t === 'mc') return grade({ ok: cur.sel === it.a, expected: it.o[it.a] });
      if (it.t === 'order' || it.t === 'listen') return grade(U.check(cur.picked.map(i => it.tiles[i]).join(' '), [it.es]));
      grade(U.check(cur.input, it.a));
    }
    function selfDone(ok) {
      const it = cur.item;
      res.selfRated++;
      if (it.t === 'speak') { P.act(S, today).speak++; LQ.app.handle(P.addXP(S, P.XP.phrase, today)); }
      if (it.t === 'describe') LQ.app.handle(P.recordAnswer(S, 'describe', { ok }, today));
      LQ.app.save(); next();
    }
    function next() { pos++; start(); }
    function finish() {
      res.score = res.answered ? res.first / res.answered : 1;
      res.xp = P.act(S, today).xp - res.xp0;
      opts.onFinish && opts.onFinish(res);
    }

    el.onclick = e => {
      const b = e.target.closest('[data-ex]'); if (!b || b.disabled) return;
      const it = cur.item, act = b.dataset.ex, i = +b.dataset.i;
      if (act === 'opt') { cur.sel = i; draw(); }
      else if (act === 'pick') { cur.picked.push(i); draw(); }
      else if (act === 'unpick') { cur.picked = cur.picked.filter(x => x !== i); draw(); }
      else if (act === 'check') check();
      else if (act === 'skip') { if (it.t === 'gender') cur.sel = null; grade({ ok: false, expected: it.t === 'mc' ? it.o[it.a] : it.t === 'order' || it.t === 'listen' ? it.es : it.a ? [].concat(it.a)[0] : '' }); }
      else if (act === 'next') next();
      else if (act === 'quit') opts.onQuit();
      else if (act === 'gender') { cur.sel = b.dataset.v; grade({ ok: b.dataset.v === it.a, expected: it.a }); }
      else if (act === 'reveal') { cur.input = (el.querySelector('#exInput') || {}).value || cur.input; cur.revealed = true; draw(); }
      else if (act === 'self') selfDone(b.dataset.ok === '1');
      else if (act === 'grade') {
        const g = +b.dataset.g, card = S.cards[it.id] || R.create(today);
        S.cards[it.id] = { ...R.grade(card, g, today), u: card.u || it.v.unit };
        res.selfRated++;
        if (g === 0 && !it.retry) queue.push({ ...it, retry: true }); // see it again before the session ends
        P.act(S, today).ex.review = P.act(S, today).ex.review || [0, 0];
        const ex = P.act(S, today).ex.review; ex[1]++; if (g) ex[0]++;
        LQ.app.handle(P.addXP(S, g ? P.XP.card : 1, today)); LQ.app.save(); next();
      }
      else if (act === 'match') {
        const side = b.dataset.side;
        if (!cur.pick || cur.pick.side === side) { cur.pick = { side, i }; cur.flash = null; draw(); return; }
        if (cur.pick.i === i) { cur.matched.push(i); if (side === 'l' || cur.pick.side === 'l') LQ.speech.speak(it.pairs[i][0]); }
        else { cur.mistakes = (cur.mistakes || 0) + 1; cur.flash = { [cur.pick.side]: cur.pick.i, [side]: i }; }
        cur.pick = null;
        if (cur.matched.length === it.pairs.length) grade({ ok: (cur.mistakes || 0) <= 1, expected: '' }); else draw();
      }
    };
    el.oninput = e => { if (e.target.id === 'exInput') { cur.input = e.target.value; const btn = el.querySelector('[data-ex="check"]'); if (btn) btn.disabled = !canCheck(); } };
    // Keys work wherever focus is (document-level) while this session is on screen.
    const NATIVE_ENTER = /^(next|reveal|self|grade|quit|skip|check)$/;
    const onKey = e => {
      if (!el.isConnected) return document.removeEventListener('keydown', onKey);
      if (e.metaKey || e.ctrlKey || e.altKey || document.getElementById('modal').open) return;
      const it = cur.item, tag = e.target.tagName, typing = tag === 'TEXTAREA' || tag === 'INPUT' || tag === 'SELECT';
      if (e.key === 'Enter') {
        // Action buttons and speaker buttons keep their native Enter → click; elsewhere Enter means Check / Continue.
        if (tag === 'TEXTAREA' || (tag === 'BUTTON' && (NATIVE_ENTER.test(e.target.dataset.ex || '') || e.target.dataset.speak))) return;
        if (typing && e.target.id !== 'exInput') return;
        e.preventDefault();
        if (cur.checked) next(); else if (!selfRated(it.t)) check();
        return;
      }
      if (typing) return;
      if (e.key === ' ' && it.t === 'card' && !cur.revealed) { e.preventDefault(); cur.revealed = true; draw(); return; }
      const n = parseInt(e.key, 10);
      if (!n || cur.checked) return;
      if (it.t === 'mc' && n <= it.o.length) { cur.sel = n - 1; draw(); }
      else if (it.t === 'gender' && n <= 2) { cur.sel = it.opts[n - 1]; grade({ ok: it.opts[n - 1] === it.a, expected: it.a }); }
      else if (it.t === 'card' && cur.revealed && n <= 3) { const g = el.querySelector(`[data-ex="grade"][data-g="${[0, 2, 3][n - 1]}"]`); if (g) g.click(); }
    };
    document.addEventListener('keydown', onKey);
    start();
    return { stop() { el.onclick = el.oninput = null; document.removeEventListener('keydown', onKey); LQ.speech.stop(); } };
  }

  // Shared end-of-session summary.
  function summary(res, { title, text = '', actions = '' }) {
    const pct = Math.round(res.score * 100);
    return `<div class="session-done card">
      <div class="done-badge">${ico('check', 'white')}</div>
      <h2>${esc(title)}</h2>${text ? `<p>${text}</p>` : ''}
      <div class="done-stats">
        ${res.answered ? `<div><b>${pct}%</b><small>${L('first-try accuracy', 'acierto al primer intento')}</small></div>` : ''}
        <div><b>+${res.xp}</b><small>XP</small></div>
        ${res.selfRated ? `<div><b>${res.selfRated}</b><small>${L('practiced', 'practicadas')}</small></div>` : ''}
      </div>
      <div class="done-actions">${actions}</div>
    </div>`;
  }

  LQ.ex = { mc, fromRaw, fromUnit, vocabItem, mixed, conj, past, cognates, falseFriends, gender, genderItem, cards, speakItems, describe, run, summary, sentenceOf };
})(typeof window !== 'undefined' ? window : globalThis);
