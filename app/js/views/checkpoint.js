/* Phase checkpoints (phases 1–4) and the B1 mock exam (phase 5), modelled on the four DELE B1 skills.
   Reading, listening and grammar are scored automatically; writing and speaking are partly self-assessed. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, VB = LQ.verbs;
  const { ico, esc, speakBtn } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  LQ.actions = LQ.actions || {}; LQ.inputs = LQ.inputs || {}; LQ.changes = LQ.changes || {};

  let cs = null;
  V.checkpoint = (param, ctx) => {
    const phase = U.clamp(parseInt(param, 10) || 1, 1, 5);
    if (!cs || cs.phase !== phase || (ctx && ctx.fresh)) cs = { phase, step: 'intro', scores: {}, text: '', self: {}, rate: {} };
    return phase === 5 ? mock() : checkpoint(phase);
  };
  const head = (title, sub) => `<section class="page-head"><div class="breadcrumb"><button type="button" class="text-link" data-go="lessons">${L('Lessons', 'Lecciones')}</button> › ${esc(title)}</div><h1>${esc(title)}</h1><p class="sub">${sub}</p></section>`;

  // ---- Phases 1–4 ----------------------------------------------------------------------------------------------
  function items(phase) {
    const units = C.units.filter(u => u.phase === phase), pool = units.flatMap(u => u.ex.map(e => ({ e, u })));
    const nRaw = phase >= 3 ? 8 : 10;
    const raw = U.sample(pool, nRaw).map(({ e, u }) => LQ.ex.fromRaw(e, u, { unit: u.id }));
    const tenses = Object.keys(VB.TENSES).filter(k => VB.TENSES[k].unit < phase * 5);
    const words = units.flatMap(u => u.vocab);
    return U.shuffle([...raw, ...LQ.ex.conj(VB.CORE, tenses, 3), ...U.sample(words, 2).map(v => LQ.ex.vocabItem(v, words)), ...(phase >= 3 ? LQ.ex.past(2) : [])]);
  }
  function checkpoint(phase) {
    const S = LQ.state, ph = C.phases[phase - 1], prev = S.checkpoints[phase], title = L(`Phase ${phase} checkpoint`, `Control de la fase ${phase}`);
    if (cs.step === 'intro') return head(title, esc(L(ph.en, ph.es)) + ' · ' + L('15 questions · about 15 minutes', '15 preguntas · unos 15 minutos')) + `<section class="card">
      <p>${L('A mix of grammar, vocabulary and verb forms from the five lessons of this phase. No second tries — it shows what is solid and what needs review.', 'Una mezcla de gramática, vocabulario y verbos de las cinco lecciones de la fase. Sin segundos intentos: muestra lo que está sólido y lo que hay que repasar.')}</p>
      ${prev ? `<p class="pill-note">${L(`Your best score: ${Math.round(prev.best * 100)}%`, `Tu mejor resultado: ${Math.round(prev.best * 100)} %`)}</p>` : ''}
      <div class="bottom-buttons"><button type="button" class="primary-btn" data-action="cp-step" data-step="run">${ico('play', 'white')}${L('Start checkpoint', 'Empezar el control')}</button></div></section>`;
    if (cs.step === 'run') return { html: head(title, esc(L(ph.en, ph.es))) + '<div id="cpSession"></div>', mount(el) {
      const s = LQ.ex.run(el.querySelector('#cpSession'), { items: items(phase), requeue: false, onFinish(res) { cs.res = res; cs.step = 'done'; LQ.app.handle(P.saveCheckpoint(LQ.state, phase, res.score, U.today())); LQ.app.save(); LQ.app.render(); }, onQuit() { cs.step = 'intro'; LQ.app.render(); } });
      return () => s.stop();
    } };
    const res = cs.res, pass = res.score >= 0.7, weak = {};
    for (const it of res.wrong) if (it.unit) weak[it.unit] = (weak[it.unit] || 0) + 1;
    const weakUnits = Object.entries(weak).sort((a, b) => b[1] - a[1]).map(([id]) => C.unitById[id]);
    return head(title, esc(L(ph.en, ph.es))) + LQ.ex.summary(res, {
      title: pass ? L(`Phase ${phase} passed!`, `¡Fase ${phase} superada!`) : L('Checkpoint saved', 'Control guardado'),
      text: pass ? L('Solid work. The next phase is open.', 'Buen trabajo. La siguiente fase está abierta.') : L('You can keep going, but review the lessons below first — 70% or more means the phase is solid.', 'Puedes seguir, pero repasa antes las lecciones de abajo: un 70 % o más indica que la fase está sólida.'),
      actions: `<button type="button" class="primary-btn" data-go="today">${ico('calendar', 'white')}${L("Today's plan", 'Plan de hoy')}</button><button type="button" class="secondary-btn" data-action="cp-step" data-step="run">${ico('refresh')}${L('Retake', 'Repetir')}</button>`
    }) + (weakUnits.length ? `<section class="card"><h2 class="card-title">${ico('refresh')}${L('Review these lessons', 'Repasa estas lecciones')}</h2>${weakUnits.map(u => `<button type="button" class="task-row" data-go="lesson/${u.id}"><div class="task-ico">${ico('book')}</div><div class="task-text"><b>${L(`Lesson ${u.n}`, `Lección ${u.n}`)}: ${esc(L(u.title, u.es))}</b><small>${esc(u.canDo)}</small></div><div class="right">${ico('chevron-right')}</div></button>`).join('')}</section>` : '');
  }

  // ---- Phase 5 · B1 mock ----------------------------------------------------------------------------------------
  const M = C.mock;
  const STEPS = ['reading', 'listening', 'grammar', 'writing', 'speaking'];
  const STEP_NAME = { reading: ['Reading', 'Comprensión de lectura'], listening: ['Listening', 'Comprensión auditiva'], grammar: ['Grammar', 'Gramática'], writing: ['Writing', 'Expresión escrita'], speaking: ['Speaking', 'Expresión oral'] };
  const CONNECTORS = ['pero', 'aunque', 'porque', 'por eso', 'sin embargo', 'además', 'luego', 'después', 'al final', 'así que', 'primero', 'cuando', 'mientras', 'de repente', 'al principio', 'por suerte'];
  const RUBRIC = [['Could not do it', 'No pude hacerlo'], ['Long pauses, some English', 'Pausas largas, algo de inglés'], ['Got the message across with errors', 'Transmití el mensaje con errores'], ['Fairly fluent, varied tenses', 'Bastante fluido, varios tiempos']];

  function writingChecks(text) {
    const words = U.words(text), n = words.length, low = ' ' + U.normalize(text) + ' ';
    const past = words.filter(w => /(é|ó|aba|abas|ábamos|aban|ía|ías|íamos|ían|ió|aron|ieron|iste|amos)$/.test(w) || /^(fui|fue|fuimos|fueron|tuve|tuvo|hice|hizo|estuve|estuvo|pude|dije|vine|era|eran|había)$/.test(w)).length;
    const conn = CONNECTORS.filter(c => low.includes(' ' + c + ' ')).length;
    const future = /\b(voy|vamos|va|van) a \w+|\w+(ré|rás|rá|remos|rán)\b|me gustaría|me encantaría|quiero|espero/.test(low);
    return [[n >= 80 && n <= 170, L(`Length: ${n} words (aim for 100–150)`, `Extensión: ${n} palabras (meta: 100–150)`)], [past >= 3, L(`Past tenses used (${past} forms found)`, `Usa tiempos pasados (${past} formas)`)], [conn >= 3, L(`Connectors (${conn}: pero, aunque, luego…)`, `Conectores (${conn}: pero, aunque, luego…)`)], [future, L('A future plan or wish (voy a…, me gustaría…)', 'Un plan o deseo futuro (voy a…, me gustaría…)')]];
  }
  const SELF = () => [L('I greeted my friend and closed the email', 'Saludé y me despedí en el correo'), L('I described a place or how I felt', 'Describí un lugar o cómo me sentí'), L('I re-read it for el/la and -o/-a agreement', 'Lo revisé (el/la y concordancia -o/-a)')];

  function mockNav() {
    const i = STEPS.indexOf(cs.step);
    return `<div class="lesson-meta"><b>${i >= 0 ? L(`Part ${i + 1} of 5`, `Parte ${i + 1} de 5`) + ' · ' + esc(L(...STEP_NAME[cs.step])) : L('B1 mock exam', 'Simulacro B1')}</b><div class="steps">${STEPS.map((s, j) => `<span class="${j < i || cs.step === 'result' ? 'on' : j === i ? 'now' : ''}"></span>`).join('')}</div>${ico('clock')}<span>~45 min</span></div>`;
  }
  function runSection(el, list, key, next) {
    const s = LQ.ex.run(el.querySelector('#mockSession'), { items: list, requeue: false, onFinish(res) { cs.scores[key] = [res.first, res.answered]; cs.step = next; LQ.app.save(); LQ.app.render(); document.getElementById('mainContent').scrollTop = 0; } });
    return () => s.stop();
  }
  const mcList = qs => qs.map(q => LQ.ex.mc(q.q, q.o, { skill: q.skill || 'reading' }));

  function mock() {
    const S = LQ.state, prev = S.checkpoints[5], title = L('B1 mock exam', 'Simulacro B1');
    const top = head(title, L('Five parts modelled on the DELE B1 skills. Practice only — not an official exam.', 'Cinco partes basadas en las destrezas del DELE B1. Es práctica, no un examen oficial.')) + mockNav();
    switch (cs.step) {
      case 'intro': return top + `<section class="card"><ul class="goal-list">${STEPS.map(s => `<li><span class="status"></span><b>${esc(L(...STEP_NAME[s]))}</b>&nbsp;· ${{ reading: L('an email, 5 questions', 'un correo, 5 preguntas'), listening: L('a voicemail, 5 questions', 'un mensaje de voz, 5 preguntas'), grammar: L('10 questions', '10 preguntas'), writing: L('an email of 100–150 words', 'un correo de 100–150 palabras'), speaking: L('4 timed tasks, out loud', '4 tareas cronometradas, en voz alta') }[s]}</li>`).join('')}</ul>
        <p class="card-sub">${L('The real DELE B1 passes at 60/100 with at least 30 in each group (written: reading + writing; oral: listening + speaking). Writing and speaking here are partly self-assessed — be honest with yourself.', 'El DELE B1 real se aprueba con 60/100 y al menos 30 en cada grupo (escrito: lectura + escritura; oral: audio + expresión oral). Aquí la escritura y la expresión oral son en parte autoevaluadas: sé honesto contigo.')}</p>
        ${prev ? `<p class="pill-note">${L(`Best score so far: ${Math.round(prev.best * 100)}%`, `Mejor resultado: ${Math.round(prev.best * 100)} %`)}</p>` : ''}
        <div class="bottom-buttons"><button type="button" class="primary-btn" data-action="cp-step" data-step="reading">${ico('play', 'white')}${L('Start the mock exam', 'Empezar el simulacro')}</button></div></section>`;
      case 'reading': return { html: top + `<div class="grid lesson-grid"><section class="card reading-text"><h2 class="card-title">${ico('book')}Lee el correo</h2><div class="letter" lang="es">${esc(M.reading.text)}</div></section><div id="mockSession"></div></div>`, mount: el => runSection(el, mcList(M.reading.questions), 'reading', 'listening') };
      case 'listening': return { html: top + `<div class="grid lesson-grid"><section class="card"><h2 class="card-title">${ico('headphones')}Escucha el mensaje</h2><p class="card-sub">${L('Listen as many times as you need (the real exam plays each audio twice). The transcript appears at the end.', 'Escucha las veces que necesites (en el examen real cada audio se oye dos veces). La transcripción aparece al final.')}</p>
          <div class="btn-row"><button type="button" class="primary-btn" data-speak="${esc(M.listening.text)}">${ico('play', 'white')}${L('Play', 'Reproducir')}</button><button type="button" class="secondary-btn" data-speak="${esc(M.listening.text)}" data-rate="0.7">${ico('turtle')}${L('Slower', 'Más lento')}</button></div></section><div id="mockSession"></div></div>`, mount: el => runSection(el, mcList(M.listening.questions.map(q => ({ ...q, skill: 'listening' }))), 'listening', 'grammar') };
      case 'grammar': return { html: top + '<div id="mockSession"></div>', mount(el) {
        const pool = C.units.filter(u => u.phase >= 3).flatMap(u => u.ex.filter(e => e.t === 'mc' || e.t === 'fill').map(e => ({ e, u })));
        return runSection(el, U.sample(pool, 10).map(({ e, u }) => LQ.ex.fromRaw(e, u)), 'grammar', 'writing');
      } };
      case 'writing': {
        const checks = writingChecks(cs.text);
        return top + `<section class="card"><h2 class="card-title">${ico('pen')}Expresión escrita</h2><p class="phrase" lang="es">${esc(M.writing.prompt)}</p>
          <textarea id="mockText" class="text-area" rows="10" lang="es" data-input="mock-text" placeholder="Hola, …">${esc(cs.text)}</textarea>${LQ.ui.accentBar('mockText')}
          <div class="grid two-grid"><div><p class="mini-title">${L('Automatic checks', 'Comprobaciones automáticas')}</p><ul class="goal-list" id="writingChecks">${checks.map(([ok, t]) => `<li><span class="status ${ok ? 'ok' : ''}">${ok ? '✓' : ''}</span>${esc(t)}</li>`).join('')}</ul></div>
            <div><p class="mini-title">${L('Check yourself', 'Revísalo tú')}</p>${SELF().map((t, i) => `<label class="check"><input type="checkbox" data-change="mock-self" value="${i}" ${cs.self[i] ? 'checked' : ''}><span>${esc(t)}</span></label>`).join('')}</div></div>
          <div class="bottom-buttons"><button type="button" class="primary-btn" data-action="mock-writing">${L('Continue to speaking', 'Continuar a la expresión oral')} ${ico('chevron-right', 'white')}</button></div></section>`;
      }
      case 'speaking': return top + M.speaking.map((t, i) => `<section class="card speak-task"><div class="card-head"><h2 class="card-title">${ico('mic')}${i + 1}. <span lang="es">${esc(t.title)}</span></h2><button type="button" class="secondary-btn" data-action="mock-timer" data-i="${i}">${ico('clock')}<span data-timer="${i}">${L('Start 2:00 timer', 'Cronómetro 2:00')}</span></button></div>
          <div class="speak-task-body">${t.image ? `<img class="task-img" src="assets/illustrations/cafe-scene.svg" alt="${L('Illustration of a café with customers and a waiter', 'Ilustración de una cafetería con clientes y un mesero')}">` : ''}<p class="phrase" lang="es">${esc(t.prompt)}</p></div>
          <p class="mini-title">${L('How did it go?', '¿Qué tal te salió?')}</p><div class="pill-row">${RUBRIC.map((r, v) => `<button type="button" class="pill ${cs.rate[i] === v ? 'active' : ''}" data-action="mock-rate" data-i="${i}" data-v="${v}">${v} · ${esc(L(...r))}</button>`).join('')}</div></section>`).join('') +
        `<div class="bottom-buttons"><button type="button" class="primary-btn" data-action="mock-finish" ${Object.keys(cs.rate).length === M.speaking.length ? '' : 'disabled'}>${ico('check', 'white')}${L('See my results', 'Ver mis resultados')}</button></div>`;
      case 'result': {
        const r = cs.result, pct = x => Math.round(x * 100);
        const row = (lab, v) => `<div class="skill-row"><span>${esc(lab)}</span>${LQ.ui.meter(v * 100, lab)}<b>${pct(v)}%</b></div>`;
        return top + `<section class="card session-done"><div class="done-badge">${ico(r.pass ? 'award' : 'check', 'white')}</div><h2>${r.pass ? L('Mock passed — B1 level in sight!', '¡Simulacro aprobado: el B1 está cerca!') : L('Mock saved', 'Simulacro guardado')}</h2>
          <p>${L(`Overall ${pct(r.overall)}% · written group ${pct(r.written)}% · oral group ${pct(r.oral)}%. DELE-style pass: 60% or more in each group.`, `Total ${pct(r.overall)} % · grupo escrito ${pct(r.written)} % · grupo oral ${pct(r.oral)} %. Aprobado estilo DELE: 60 % o más en cada grupo.`)}</p></section>
          <section class="card">${row(L('Reading + grammar', 'Lectura + gramática'), r.reading)}${row(L('Listening', 'Comprensión auditiva'), r.listening)}${row(L('Writing (checks + self-review)', 'Escritura (comprobaciones + autoevaluación)'), r.writing)}${row(L('Speaking (self-rated)', 'Expresión oral (autoevaluada)'), r.speaking)}
          <p class="card-sub">${L('For an official certificate, the Instituto Cervantes runs the DELE B1 several times a year, including at US exam centres.', 'Para un certificado oficial, el Instituto Cervantes organiza el DELE B1 varias veces al año, también en centros de Estados Unidos.')}</p>
          <div class="bottom-buttons"><button type="button" class="primary-btn" data-go="progress">${ico('chart', 'white')}${L('See progress', 'Ver progreso')}</button><button type="button" class="secondary-btn" data-action="cp-step" data-step="intro">${ico('refresh')}${L('Retake later', 'Repetir más tarde')}</button></div></section>`;
      }
    }
    return top;
  }

  let timer = null;
  Object.assign(LQ.actions, {
    'cp-step'(b) { const step = b.dataset.step; cs = { ...cs, step, ...(step === 'intro' || step === 'reading' ? { scores: {}, text: cs.text, self: {}, rate: {} } : {}) }; LQ.app.render(); document.getElementById('mainContent').scrollTop = 0; },
    'mock-writing'() {
      const S = LQ.state, checks = writingChecks(cs.text), auto = checks.filter(c => c[0]).length, self = Object.values(cs.self).filter(Boolean).length;
      cs.scores.writing = [auto + self, 7];
      if (U.words(cs.text).length) S.writing.push({ date: U.today(), words: U.words(cs.text).length, text: cs.text.slice(0, 4000) });
      cs.step = 'speaking'; LQ.app.save(); LQ.app.render(); document.getElementById('mainContent').scrollTop = 0;
    },
    'mock-timer'(b) {
      const i = b.dataset.i; clearInterval(timer); let left = 120;
      const show = () => { const el = document.querySelector(`[data-timer="${i}"]`); if (!el) return clearInterval(timer); el.textContent = left > 0 ? `${Math.floor(left / 60)}:${String(left % 60).padStart(2, '0')}` : L("Time's up", 'Tiempo'); };
      show(); timer = setInterval(() => { left--; show(); if (left <= 0) clearInterval(timer); }, 1000);
    },
    'mock-rate'(b) { cs.rate[b.dataset.i] = +b.dataset.v; LQ.app.render(); },
    'mock-finish'() {
      const S = LQ.state, sc = cs.scores, f = k => sc[k] && sc[k][1] ? sc[k][0] / sc[k][1] : 0;
      const reading = (sc.reading && sc.grammar) ? (sc.reading[0] + sc.grammar[0]) / (sc.reading[1] + sc.grammar[1]) : f('reading');
      const speaking = Object.values(cs.rate).reduce((a, b) => a + b, 0) / (3 * M.speaking.length);
      const listening = f('listening'), writing = f('writing');
      const written = (reading + writing) / 2, oral = (listening + speaking) / 2, overall = (reading + listening + writing + speaking) / 4;
      cs.result = { reading, listening, writing, speaking, written, oral, overall, pass: written >= 0.6 && oral >= 0.6 };
      clearInterval(timer);
      LQ.app.handle(P.saveCheckpoint(S, 5, overall, U.today())); LQ.app.handle(P.checkAchievements(S, U.today()));
      cs.step = 'result'; LQ.app.save(); LQ.app.render(); document.getElementById('mainContent').scrollTop = 0;
    }
  });
  LQ.inputs['mock-text'] = el => {
    cs.text = el.value;
    const list = document.getElementById('writingChecks');
    if (list) list.innerHTML = writingChecks(cs.text).map(([ok, t]) => `<li><span class="status ${ok ? 'ok' : ''}">${ok ? '✓' : ''}</span>${esc(t)}</li>`).join('');
  };
  LQ.changes['mock-self'] = el => { cs.self[el.value] = el.checked; };
})(typeof window !== 'undefined' ? window : globalThis);
