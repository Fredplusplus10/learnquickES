/* Practice hub and trainers: verb gym, fui vs. iba, cognates, false friends, el/la, speaking, language island,
   circumlocution, mixed review and spaced repetition. Each trainer = intro/reference + a session + a summary. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, VB = LQ.verbs, R = LQ.srs;
  const { ico, esc, speakBtn } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  LQ.actions = LQ.actions || {}; LQ.changes = LQ.changes || {};

  const completedUnits = S => C.units.filter(u => S.units[u.id]);
  const unlockedTenses = S => Object.keys(VB.TENSES).filter(k => VB.TENSES[k].unit === 0 || S.units[C.units[VB.TENSES[k].unit].id]);
  function finishTask(task) { if (!task) return false; const ev = P.markTask(LQ.state, task, U.today()); LQ.app.handle(ev); LQ.app.save(); return P.taskDone(LQ.state, P.planStatus(LQ.state).n, task) || ev.length > 0; }

  // ---- Trainer catalogue -------------------------------------------------------------------------------------
  const TR = {
    verbs: {
      icon: 'bolt', tone: '', trick: 1, skill: 'verbs', task: 'verbs', title: ['Verb gym · 80/20', 'Gimnasio de verbos · 80/20'],
      desc: ['ser, estar, tener, hacer, poder, querer, ir, decir — in the tenses you have unlocked.', 'ser, estar, tener, hacer, poder, querer, ir, decir, en los tiempos que ya desbloqueaste.'],
      init: S => ({ set: 'core', tenses: unlockedTenses(S), table: 'ser' }),
      build: (S, o) => LQ.ex.conj(o.set === 'core' ? VB.CORE : Object.keys(VB.VERBS), o.tenses.length ? o.tenses : ['pres'], 10, S.profile.vosotros),
      intro: verbsIntro
    },
    past: {
      icon: 'clock', tone: 'blue', trick: 5, skill: 'past', task: 'past', title: ['Fui vs. Iba', 'Fui vs. iba'],
      desc: ['Pretérito or imperfecto? The skill that separates A2 from B1.', '¿Pretérito o imperfecto? La habilidad que separa A2 de B1.'],
      build: () => LQ.ex.past(10), intro: pastIntro
    },
    cognates: {
      icon: 'sparkles', tone: '', trick: 1, skill: 'cognates', task: 'cognates', title: ['Cognates lab', 'Laboratorio de cognados'],
      desc: ['Turn English words into Spanish with a handful of ending rules.', 'Convierte palabras inglesas en español con unas pocas reglas.'],
      build: () => LQ.ex.cognates(10), intro: cognatesIntro
    },
    falsefriends: {
      icon: 'lightbulb', tone: '', trick: 1, skill: 'falsefriends', task: 'falsefriends', title: ['False friends', 'Falsos amigos'],
      desc: ['Words that look English but mean something else — embarazada!', 'Palabras que parecen inglés pero significan otra cosa: ¡embarazada!'],
      build: () => LQ.ex.falseFriends(8), intro: falseFriendsIntro
    },
    gender: {
      icon: 'cards', tone: 'purple', trick: 5, skill: 'gender', task: 'gender', title: ['El / La trainer', 'Entrenador el / la'],
      desc: ['Never learn "mapa" — learn "el mapa". Rules and famous exceptions.', 'Nunca aprendas «mapa»: aprende «el mapa». Reglas y excepciones famosas.'],
      build: () => LQ.ex.gender(12), intro: genderIntro
    },
    speak: {
      icon: 'mic', tone: 'green', trick: 4, skill: 'speaking', task: 'speak', title: ['Speaking: shadowing', 'Hablar: shadowing'],
      desc: ['Listen, repeat out loud, record yourself. Train your mouth every day.', 'Escucha, repite en voz alta, grábate. Entrena tu boca cada día.'],
      init: () => ({ group: 0 }), build: speakItems, intro: speakIntro
    },
    describe: {
      icon: 'palm', tone: 'green', trick: 2, skill: 'describe', task: 'describe', title: ['Describe it in Spanish', 'Descríbelo en español'],
      desc: ["Don't know a word? Describe it. The core language-island skill.", '¿No sabes una palabra? Descríbela. La habilidad clave de la isla.'],
      build: () => LQ.ex.describe(5), intro: describeIntro
    },
    mixed: {
      icon: 'target', tone: 'purple', trick: 3, skill: 'grammar', task: 'mixed', auto: true, title: ['Mixed practice', 'Práctica mixta'],
      desc: ['Exercises from every unit you have completed, shuffled.', 'Ejercicios de todas tus unidades completadas, mezclados.'],
      build: S => LQ.ex.mixed(completedUnits(S), 10)
    },
    review: {
      icon: 'refresh', tone: 'blue', trick: 1, skill: 'review', task: 'review', auto: true, title: ['Review (spaced repetition)', 'Repaso espaciado'],
      desc: ['Words come back just before you forget them.', 'Las palabras vuelven justo antes de que las olvides.'],
      build: S => LQ.ex.cards(R.queue(S.cards, U.today(), S.profile.reviewLimit))
    }
  };

  // ---- Hub ---------------------------------------------------------------------------------------------------
  V.practice = (key, ctx) => key ? (key === 'island' ? islandView() : TR[key] ? trainerView(key, ctx) : V.practice()) : hub();
  function hub() {
    const S = LQ.state, T = P.totals(S, U.today()), st = P.planStatus(S);
    const card = (key, extra = '') => { const d = TR[key], acc = T.accuracy(d.skill), inPlan = !st.finished && st.tasks.includes(key);
      return `<button type="button" class="trainer-card" data-go="practice/${key}"><div class="task-ico ${d.tone}">${ico(d.icon)}</div><b>${esc(L(...d.title))}</b><p>${esc(L(...d.desc))}</p>
        <div class="trainer-foot"><span class="trick-chip">${L('Trick', 'Truco')} ${d.trick}</span>${extra}${acc !== null ? `<small>${Math.round(acc * 100)}% ${L('accuracy', 'acierto')}</small>` : ''}${inPlan ? `<span class="state-badge ${P.taskDone(S, st.n, key) ? 'done' : 'current'}">${P.taskDone(S, st.n, key) ? '✓ ' + L('today', 'hoy') : L('In today\'s plan', 'En el plan de hoy')}</span>` : ''}</div></button>`; };
    return `<section class="page-head"><h1>${L('Practice', 'Práctica')}</h1><p class="sub">${L('Short, focused trainers built on the five tricks. Your daily plan picks some for you; use any of them whenever you want extra reps.', 'Entrenadores cortos basados en los cinco trucos. Tu plan diario elige algunos; usa cualquiera cuando quieras más práctica.')}</p></section>
    <section class="grid two-grid">
      <button type="button" class="trainer-card wide" data-go="practice/review"><div class="task-ico blue">${ico('refresh')}</div><div><b>${L('Review your words', 'Repasa tus palabras')}</b><p>${T.words.due ? L(`${T.words.due} cards are due today.`, `Hoy tienes ${T.words.due} tarjetas pendientes.`) : L('Nothing due right now — come back tomorrow.', 'Nada pendiente ahora: vuelve mañana.')} ${L(`${T.words.remembered} of ${T.words.seen} words remembered.`, `${T.words.remembered} de ${T.words.seen} palabras recordadas.`)}</p></div>${ico('chevron-right')}</button>
      <button type="button" class="trainer-card wide island" data-go="practice/island"><div class="task-ico green">${ico('palm')}</div><div><b>${L('Language island', 'Isla lingüística')}</b><p>${L('Trick 2: a timed hour with zero English. The whole app switches to Spanish.', 'Truco 2: una hora cronometrada sin inglés. Toda la app cambia a español.')}</p></div>${ico('chevron-right')}</button>
    </section>
    <h2 class="section-title">${ico('bolt')}${L('Trainers', 'Entrenadores')}</h2>
    <section class="trainer-grid">${['verbs', 'past', 'speak', 'cognates', 'gender', 'falsefriends', 'describe', 'mixed'].map(k => card(k)).join('')}</section>`;
  }

  // ---- Generic trainer page -----------------------------------------------------------------------------------
  let ps = null;
  function trainerView(key, ctx) {
    const S = LQ.state, d = TR[key];
    if (!ps || ps.key !== key || (ctx && ctx.fresh)) ps = { key, phase: d.auto ? 'run' : 'intro', o: d.init ? d.init(S) : {} };
    const head = `<section class="page-head"><div class="breadcrumb"><button type="button" class="text-link" data-go="practice">${L('Practice', 'Práctica')}</button> › ${esc(L(...d.title))}</div>
      <h1>${esc(L(...d.title))}</h1><p class="sub">${esc(L(...d.desc))} <span class="trick-chip">${L('Trick', 'Truco')} ${d.trick}</span></p></section>`;
    let main = '';
    if (ps.phase === 'intro') main = `${d.intro ? d.intro(S, ps.o) : ''}`;
    else if (ps.phase === 'run') main = `<div id="trainerSession"></div>`;
    else main = doneView(d);
    return {
      html: head + main,
      mount(el) {
        if (ps.phase !== 'run') return;
        const items = d.build(S, ps.o), box = el.querySelector('#trainerSession');
        if (!items.length) { box.innerHTML = emptyFor(key); if (key === 'review') finishTask('review'); return; }
        ps.session = LQ.ex.run(box, {
          items,
          onFinish(res) { ps.res = res; ps.phase = 'done'; ps.marked = finishTask(d.task); LQ.app.render(); },
          onQuit() { ps.phase = d.auto ? 'quit' : 'intro'; if (d.auto) LQ.app.go('practice'); else LQ.app.render(); }
        });
        return () => ps && ps.session && ps.session.stop();
      }
    };
  }
  function emptyFor(key) {
    if (key === 'review') {
      const next = Object.values(LQ.state.cards).map(c => c.due).sort()[0];
      return LQ.ui.emptyState('check', L('All caught up!', '¡Todo al día!'), next ? L(`Your next cards are due ${U.fmtDate(next, 'en')}. New words arrive with each lesson.`, `Tus próximas tarjetas vencen el ${U.fmtDate(next, 'es')}. Cada lección añade palabras nuevas.`) : L('Finish a lesson to add its words to your deck.', 'Termina una lección para añadir sus palabras a tu mazo.'), `<button type="button" class="primary-btn" data-go="today">${L("Back to today's plan", 'Volver al plan de hoy')}</button>`, next ? 'empty-caught-up' : 'empty-nothing-yet');
    }
    return LQ.ui.emptyState('book', L('Nothing to practice yet', 'Aún no hay nada que practicar'), L('Complete your first lesson to unlock this trainer.', 'Completa tu primera lección para desbloquear este entrenador.'), `<button type="button" class="primary-btn" data-go="lessons">${L('Go to lessons', 'Ir a las lecciones')}</button>`, 'empty-nothing-yet');
  }
  function doneView(d) {
    const res = ps.res, wrong = res.wrong.slice(0, 6);
    const review = wrong.length ? `<section class="card"><h2 class="card-title">${ico('refresh')}${L('Worth another look', 'Para repasar')}</h2><ul class="fix-list">${wrong.map(it => {
      const answer = it.t === 'mc' ? it.o[it.a] : LQ.ex.sentenceOf(it) || [].concat(it.a)[0];
      const q = it.t === 'conj' ? `${it.verb} · ${L(VB.TENSES[it.tense].en, VB.TENSES[it.tense].es)}` : it.q || it.en || it.word || '';
      return `<li><span>${esc(q)}</span><b lang="es">${esc(answer)}</b>${answer ? speakBtn(answer) : ''}${it.why ? `<small>${esc(it.why)}</small>` : ''}</li>`;
    }).join('')}</ul></section>` : '';
    return LQ.ex.summary(res, {
      title: L('Session complete!', '¡Sesión completada!'),
      text: ps.marked ? L("✓ This task is checked off in today's plan.", '✓ Esta tarea quedó marcada en el plan de hoy.') : '',
      actions: `<button type="button" class="primary-btn" data-go="today">${ico('calendar', 'white')}${L("Today's plan", 'Plan de hoy')}</button><button type="button" class="secondary-btn" data-action="trainer-again">${ico('refresh')}${L('Again', 'Otra vez')}</button><button type="button" class="secondary-btn" data-go="practice">${L('All trainers', 'Todos los entrenadores')}</button>`
    }) + review;
  }
  const startBtn = (label) => `<div class="bottom-buttons"><button type="button" class="primary-btn" data-action="trainer-start">${ico('play', 'white')}${label || L('Start session', 'Empezar sesión')}</button></div>`;

  // ---- Intros / reference panels -------------------------------------------------------------------------------
  function verbsIntro(S, o) {
    const unlocked = unlockedTenses(S), tenses = Object.entries(VB.TENSES);
    const tbl = o.table, shown = o.tenses.length ? o.tenses : ['pres'];
    const persons = [0, 1, 2, 3, 5].concat(S.profile.vosotros ? [4] : []).sort();
    return `<section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('settings')}${L('Session setup', 'Configura la sesión')}</h2>
        <p class="mini-title">${L('Verbs', 'Verbos')}</p><div class="pill-row">
          <button type="button" class="pill ${o.set === 'core' ? 'active' : ''}" data-action="verb-set" data-v="core">${L('80/20 core (8)', 'Núcleo 80/20 (8)')}</button>
          <button type="button" class="pill ${o.set === 'all' ? 'active' : ''}" data-action="verb-set" data-v="all">${L(`All ${Object.keys(VB.VERBS).length} verbs`, `Los ${Object.keys(VB.VERBS).length} verbos`)}</button></div>
        <p class="mini-title">${L('Tenses', 'Tiempos')}</p><div class="check-list">${tenses.map(([k, t]) => {
          const open = unlocked.includes(k);
          return `<label class="check ${open ? '' : 'locked'}"><input type="checkbox" data-change="verb-tense" value="${k}" ${o.tenses.includes(k) ? 'checked' : ''} ${open ? '' : 'disabled'}><span>${esc(L(t.en, t.es))}</span>${open ? `<small lang="es">${esc(t.cue)}</small>` : `<small>${ico('lock')} ${L(`after Lesson ${t.unit + 1}`, `tras la lección ${t.unit + 1}`)}</small>`}</label>`;
        }).join('')}</div>
        <label class="check"><input type="checkbox" data-change="vosotros" ${S.profile.vosotros ? 'checked' : ''}><span>${L('Include vosotros (Spain only)', 'Incluir vosotros (solo España)')}</span></label>
        ${startBtn()}</div>
      <div class="card"><div class="card-head"><h2 class="card-title">${ico('book')}${L('Verb tables', 'Tablas de verbos')}</h2>
        <select data-change="verb-table" aria-label="${L('Verb', 'Verbo')}">${Object.keys(VB.VERBS).map(v => `<option value="${v}" ${v === tbl ? 'selected' : ''}>${v}${VB.VERBS[v].core ? ' ★' : ''}</option>`).join('')}</select></div>
        <p class="card-sub">${esc(VB.VERBS[tbl].en)} · ${L('showing the tenses selected on the left', 'tiempos seleccionados a la izquierda')}</p>
        <div class="table-wrap"><table class="g-table has-labels"><thead><tr><th></th>${shown.map(k => `<th>${esc(L(VB.TENSES[k].en, VB.TENSES[k].es))}</th>`).join('')}</tr></thead><tbody>
          ${persons.map(p => `<tr><td>${esc(VB.PERSONS[p].join(' / '))}</td>${shown.map(k => `<td lang="es">${esc(VB.form(tbl, k, p))}</td>`).join('')}</tr>`).join('')}</tbody></table></div>
        <button type="button" class="secondary-btn" data-speak="${esc(persons.map(p => VB.PERSONS[p][0] + ' ' + VB.form(tbl, shown[0], p)).join(', '))}">${ico('speaker')}${L('Listen', 'Escuchar')}</button></div></section>`;
  }
  function pastIntro() {
    return `<section class="card"><div class="hack">${ico('lightbulb')}<p>${L('Think of a movie. The IMPERFECTO is the scene: what was going on, what things were like, habits. The PRETÉRITO is the action: what happened, once, at a moment.', 'Piensa en una película. El IMPERFECTO es la escena: lo que pasaba, cómo eran las cosas, los hábitos. El PRETÉRITO es la acción: lo que pasó, una vez, en un momento.')}</p></div>
      <div class="table-wrap"><table class="g-table"><thead><tr><th>${L('Imperfecto — scene', 'Imperfecto — escena')}</th><th>${L('Pretérito — event', 'Pretérito — acción')}</th></tr></thead><tbody>
        <tr><td lang="es">De niño <b>iba</b> a la playa.</td><td lang="es">Ayer <b>fui</b> a la playa.</td></tr>
        <tr><td lang="es"><b>Llovía</b> y <b>hacía</b> frío…</td><td lang="es">…cuando <b>sonó</b> el teléfono.</td></tr>
        <tr><td lang="es"><b>Eran</b> las diez. <b>Tenía</b> veinte años.</td><td lang="es"><b>Llegué</b>, <b>comí</b> y <b>salí</b>.</td></tr>
        <tr><td lang="es">siempre · todos los días · mientras</td><td lang="es">ayer · una vez · de repente</td></tr></tbody></table></div>
      ${startBtn()}</section>`;
  }
  function cognatesIntro() {
    return `<section class="card"><p class="card-sub">${L('About a third of English words have a Spanish cognate. These rules turn thousands of them into Spanish — stress the ending and keep the vowels pure.', 'Cerca de un tercio de las palabras inglesas tiene un cognado en español. Estas reglas convierten miles de ellas: acentúa la terminación y mantén las vocales puras.')}</p>
      <div class="rule-grid">${C.cognateRules.map(([en, es, list]) => `<div class="rule"><b>${esc(en)} → ${esc(es)}</b><span lang="es">${list.slice(0, 3).map(([, s]) => esc(s)).join(', ')}</span></div>`).join('')}</div>${startBtn()}</section>`;
  }
  function falseFriendsIntro() {
    return `<section class="card"><div class="table-wrap"><table class="g-table"><thead><tr><th>${L('Spanish', 'Español')}</th><th>${L('Really means', 'Significa')}</th><th>${L('Not…', 'No es…')}</th><th>${L('For that, say', 'Para eso, di')}</th></tr></thead><tbody>
      ${C.falseFriends.map(([es, ok, bad, real]) => `<tr><td lang="es"><b>${esc(es)}</b> ${speakBtn(es)}</td><td>${esc(ok)}</td><td class="muted">${esc(bad)}</td><td lang="es">${esc(real)}</td></tr>`).join('')}</tbody></table></div>${startBtn()}</section>`;
  }
  function genderIntro() {
    return `<section class="card"><p class="card-sub">${L('Article colours: ', 'Colores de artículo: ')}<span class="art art-m">el</span> ${L('masculine', 'masculino')} · <span class="art art-f">la</span> ${L('feminine', 'femenino')}. ${L('Say the article out loud every time.', 'Di siempre el artículo en voz alta.')}</p>
      <div class="rule-grid">${C.genderRules.map(([rule, ex, note]) => `<div class="rule"><b>${esc(rule)}</b><span lang="es">${esc(ex)}</span>${note ? `<small>${esc(note)}</small>` : ''}</div>`).join('')}</div>${startBtn()}</section>`;
  }
  function speakItems(S) {
    const done = completedUnits(S), phrases = done.flatMap(u => u.speak);
    const group = C.pronunciation[Math.floor(U.seeded(U.today())() * C.pronunciation.length)];
    return LQ.ex.speakItems(U.sample(phrases, Math.min(4, phrases.length)).concat(U.sample(group.items, done.length ? 2 : 6)));
  }
  function speakIntro(S, o) {
    const g = C.pronunciation[o.group];
    return `<section class="card"><h2 class="card-title">${ico('mic')}${L("Today's speaking session", 'Sesión de hoy')}</h2>
      <p class="card-sub">${L('6 phrases from your lessons and the pronunciation gym. For each: listen, repeat out loud 2–3 times, and record yourself if your browser allows it.', '6 frases de tus lecciones y del gimnasio de pronunciación. Para cada una: escucha, repite en voz alta 2 o 3 veces y grábate si tu navegador lo permite.')}</p>
      ${!LQ.speech.canRecord() && !LQ.speech.hasRecognition ? `<p class="pill-note">${L('Recording is not available in this browser — "Listen & repeat" works everywhere. Safari usually allows recording.', 'La grabación no está disponible en este navegador; «Escucha y repite» funciona siempre. Safari suele permitir grabar.')}</p>` : ''}
      ${startBtn()}</section>
    <section class="card"><h2 class="card-title">${ico('waveform')}${L('Pronunciation gym', 'Gimnasio de pronunciación')}</h2>
      <div class="pill-row">${C.pronunciation.map((p, i) => `<button type="button" class="pill ${i === o.group ? 'active' : ''}" data-action="pron-group" data-i="${i}">${esc(p.title)}</button>`).join('')}</div>
      <div class="coach-tip">${esc(g.tip)}</div>${g.items.map(x => LQ.ui.shadow(x)).join('')}</section>
    <section class="card"><h2 class="card-title">${ico('sparkles')}${L('Tongue twisters (trabalenguas)', 'Trabalenguas')}</h2><p class="card-sub">${L('Start slowly, then speed up. Exaggerate every R.', 'Empieza despacio y luego acelera. Exagera cada R.')}</p>
      ${C.tongueTwisters.map(t => `<div class="example">${speakBtn(t)}<button type="button" class="speak-button" data-speak="${esc(t)}" data-rate="0.65" aria-label="${L('Slow', 'Lento')}">${ico('turtle')}</button><b lang="es">${esc(t)}</b></div>`).join('')}</section>`;
  }
  function describeIntro() {
    return `<section class="card"><div class="hack">${ico('lightbulb')}<p>${L("When you don't know a word, don't switch to English: describe it. This keeps you inside Spanish and is exactly what fluent speakers do.", 'Cuando no sepas una palabra, no cambies al inglés: descríbela. Así te quedas dentro del español, y es lo que hacen los hablantes con fluidez.')}</p></div>
      <p class="mini-title">${L('Useful frames', 'Frases útiles')}</p><div class="frames">${C.describeFrames.map(f => `<span class="frame" lang="es">${esc(f)}</span>`).join('')}</div>${startBtn()}</section>`;
  }

  // ---- Language island ------------------------------------------------------------------------------------------
  const IDEAS = ['Escucha un pódcast en español (No Hay Tos, Dreaming Spanish).', 'Describe en voz alta lo que haces: «Ahora estoy cocinando…».', 'Piensa en español: planea tu día y tu semana.', 'Lee noticias en español (BBC Mundo, CNN en Español).', 'Escribe tu lista de compras o tus tareas en español.', 'Si no sabes una palabra, descríbela: «Es una cosa que sirve para…».'];
  LQ.island = LQ.island || { active: false };
  const islandMin = () => LQ.island.active ? Math.floor((Date.now() - LQ.island.start) / 60000) : 0;
  LQ.island.clock = () => { const s = Math.floor((Date.now() - LQ.island.start) / 1000); return `${Math.floor(s / 60)}:${String(s % 60).padStart(2, '0')}`; };
  function islandView() {
    const S = LQ.state, a = S.activity[U.today()] || {}, total = P.totals(S, U.today()).islandMin;
    if (!LQ.island.active) return `<section class="page-head"><div class="breadcrumb"><button type="button" class="text-link" data-go="practice">${L('Practice', 'Práctica')}</button> › ${L('Language island', 'Isla lingüística')}</div><h1>${ico('palm', 'h1-ico')} ${L('Language island', 'Isla lingüística')}</h1>
      <p class="sub">${L('Trick 2: at least one hour a day with zero English. Start the timer, then live in Spanish: the app interface switches to Spanish until you end the session.', 'Truco 2: al menos una hora al día sin inglés. Inicia el cronómetro y vive en español: la interfaz cambia a español hasta que termines.')}</p></section>
      <section class="grid two-grid"><div class="card"><h2 class="card-title">${ico('clock')}${L('Start a session', 'Empieza una sesión')}</h2>
        <p class="metric-row"><b>${Math.round(a.island || 0)} / 60 min</b> <small>${L('today', 'hoy')}</small></p>${LQ.ui.meter((a.island || 0) / 60 * 100, L('Island today', 'Isla hoy'))}
        <p class="card-sub">${L(`${(total / 60).toFixed(1)} hours on the island in total. Minutes count toward your immersion goal and your streak.`, `${(total / 60).toFixed(1)} horas en la isla en total. Los minutos cuentan para tu meta de inmersión y tu racha.`)}</p>
        <button type="button" class="primary-btn" data-action="island-start">${ico('palm', 'white')}${L('Start the island (Spanish only)', 'Empezar la isla (solo español)')}</button></div>
        <div class="card"><h2 class="card-title">${ico('lightbulb')}${L('Ideas for your hour', 'Ideas para tu hora')}</h2><ul class="idea-list">${IDEAS.map(i => `<li lang="es">${esc(i)}</li>`).join('')}</ul></div></section>`;
    const prompt = C.describe[LQ.island.prompt % C.describe.length];
    return `<section class="island-live card"><div class="island-clock">${ico('palm')}<b data-island-clock>${LQ.island.clock()}</b><small>/ 60:00</small></div>
      <h1>Estás en la isla. ¡Solo español!</h1><p class="sub">Si no sabes una palabra, descríbela. Puedes usar toda la app: se queda en español hasta que termines.</p>
      <div class="bottom-buttons"><button type="button" class="secondary-btn" data-go="immersion">${ico('headphones')}Registrar escucha</button><button type="button" class="primary-btn" data-action="island-stop">${ico('check', 'white')}Terminar la isla</button></div></section>
    <section class="grid two-grid"><div class="card"><h2 class="card-title">${ico('sparkles')}Descríbelo en español</h2>
        <p class="card-sub">Explica esto sin usar la palabra:</p><p class="ex-big">${esc(prompt[0])}</p>
        <div class="frames">${C.describeFrames.map(f => `<span class="frame" lang="es">${esc(f)}</span>`).join('')}</div>
        <details class="model"><summary>Ver un modelo</summary><p lang="es">${esc(prompt[2])} ${speakBtn(prompt[2])}</p><p><b>La palabra:</b> <span lang="es">${esc(prompt[1])}</span></p></details>
        <button type="button" class="secondary-btn" data-action="island-prompt">${ico('refresh')}Otra palabra</button></div>
      <div class="card"><h2 class="card-title">${ico('lightbulb')}Ideas para tu hora</h2><ul class="idea-list">${IDEAS.map(i => `<li lang="es">${esc(i)}</li>`).join('')}</ul></div></section>`;
  }

  Object.assign(LQ.actions, {
    'trainer-start'() { ps.phase = 'run'; LQ.app.render(); },
    'trainer-again'() { ps.phase = 'run'; ps.res = null; LQ.app.render(); },
    'verb-set'(b) { ps.o.set = b.dataset.v; LQ.app.render(); },
    'pron-group'(b) { ps.o.group = +b.dataset.i; LQ.app.render(); },
    'island-start'() {
      LQ.island = { ...LQ.island, active: true, start: Date.now(), prompt: Math.floor(Math.random() * C.describe.length) };
      LQ.state.islandStart = LQ.island.start; LQ.app.save(); LQ.app.render();
      LQ.ui.toast('¡Bienvenido a la isla! Desde ahora, solo español.');
    },
    'island-prompt'() { LQ.island.prompt++; LQ.app.render(); },
    'island-stop'() {
      const S = LQ.state, min = Math.min(180, islandMin());
      LQ.island.active = false; delete S.islandStart;
      if (min >= 1) LQ.app.handle(P.logMinutes(S, 'island', min, U.today()));
      LQ.app.save(); LQ.app.render();
      LQ.ui.toast(min >= 1 ? L(`${min} minutes on the island. ¡Muy bien!`, `${min} minutos en la isla. ¡Muy bien!`) : L('Island session ended (under a minute, not logged).', 'Sesión terminada (menos de un minuto, no se registró).'));
      LQ.ui.flushCelebrations();
    }
  });
  Object.assign(LQ.changes, {
    'verb-tense'(el) { const set = new Set(ps.o.tenses); el.checked ? set.add(el.value) : set.delete(el.value); ps.o.tenses = Object.keys(VB.TENSES).filter(k => set.has(k)); LQ.app.render(); },
    'verb-table'(el) { ps.o.table = el.value; LQ.app.render(); },
    vosotros(el) { LQ.state.profile.vosotros = el.checked; LQ.app.save(); LQ.app.render(); }
  });
})(typeof window !== 'undefined' ? window : globalThis);
