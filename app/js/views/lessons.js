/* Course map and the lesson player: vocabulary → grammar hack → dialogue → practice → speaking → summary. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content;
  const { ico, esc, wordRow, wordHTML, speakBtn, genderTag } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  const TRICKS = { 1: ['80/20 & cognates', '80/20 y cognados'], 2: ['No English', 'Sin inglés'], 3: ['Comprehensible input', 'Input comprensible'], 4: ['Speak from day one', 'Habla desde el día uno'], 5: ['Hack the grammar', 'Hackea la gramática'] };

  V.lessons = () => {
    const S = LQ.state, st = P.planStatus(S), todayUnit = st.unit && st.unit.id;
    return `<section class="page-head"><h1>${L('Lessons', 'Lecciones')}</h1><p class="sub">${L('25 units in 5 phases, from A2 foundations to real-world B1. Your daily plan picks the next one, but you can open any unit.', '25 unidades en 5 fases, de los fundamentos A2 al B1 real. Tu plan diario elige la siguiente, pero puedes abrir cualquiera.')}</p></section>
    ${C.phases.map(ph => {
      const units = C.units.filter(u => u.phase === ph.n), done = units.filter(u => S.units[u.id]).length, cp = S.checkpoints[ph.n];
      return `<section class="card phase-block"><div class="phase-head">${ico(ph.icon)}<div><p class="eyebrow">${L('Phase', 'Fase')} ${ph.n}</p><h2>${esc(L(ph.en, ph.es))}</h2><p class="card-sub">${esc(ph.desc)}</p></div><span class="tag">${done}/5</span></div>
        <div class="unit-grid">${units.map(u => {
          const r = S.units[u.id], state = r ? 'done' : u.id === todayUnit ? 'today' : '';
          return `<button type="button" class="unit-card ${state}" data-go="lesson/${u.id}"><span class="unit-n">${u.n}</span><b>${esc(u.title)}</b><small lang="es">${esc(u.es)}</small><p>${esc(u.canDo)}</p>
            <span class="state-badge ${r ? 'done' : state === 'today' ? 'current' : ''}">${r ? `✓ ${Math.round(r.best * 100)}%` : state === 'today' ? L('Today', 'Hoy') : L('Not started', 'Sin empezar')}</span></button>`;
        }).join('')}
          <button type="button" class="unit-card checkpoint ${cp ? 'done' : ''}" data-go="checkpoint/${ph.n}"><span class="unit-n">${ico('award')}</span><b>${ph.n === 5 ? L('B1 mock exam', 'Simulacro B1') : L(`Phase ${ph.n} checkpoint`, `Control de la fase ${ph.n}`)}</b><p>${ph.n === 5 ? L('Reading, listening, grammar, writing and speaking.', 'Lectura, audio, gramática, escritura y expresión oral.') : L('15 questions to check this phase.', '15 preguntas para comprobar la fase.')}</p>
            <span class="state-badge ${cp ? 'done' : ''}">${cp ? `✓ ${Math.round(cp.best * 100)}%` : L('Not taken', 'Pendiente')}</span></button>
        </div></section>`;
    }).join('')}`;
  };

  // ---- Lesson player ---------------------------------------------------------------------------------------
  const STEPS = ['vocab', 'grammar', 'dialogue', 'practice', 'speak', 'done'];
  const STEP_LABEL = { vocab: ['Vocabulary', 'Vocabulario'], grammar: ['Grammar hack', 'Truco gramatical'], dialogue: ['Dialogue', 'Diálogo'], practice: ['Practice', 'Práctica'], speak: ['Speaking', 'Hablar'], done: ['Summary', 'Resumen'] };
  let ls = null; // in-memory state of the open lesson

  function table(t) { return `<div class="table-wrap"><table class="g-table${t.head[0] ? '' : ' has-labels'}"><thead><tr>${t.head.map(h => `<th>${esc(h)}</th>`).join('')}</tr></thead><tbody>${t.rows.map(r => `<tr>${r.map((c, i) => `<td ${i ? 'lang="es"' : ''}>${esc(c)}</td>`).join('')}</tr>`).join('')}</tbody></table></div>`; }
  function stepBody(u) {
    switch (ls.step) {
      case 'vocab': return `<section class="card"><div class="card-head"><h2 class="card-title">${ico('book')}${L('New words', 'Palabras nuevas')}</h2><button type="button" class="secondary-btn" data-action="play-all" data-list="vocab">${ico('speaker')}${L('Play all', 'Escuchar todas')}</button></div>
        <p class="card-sub">${L('Listen and repeat each word out loud. Nouns always come with el/la — learn them together.', 'Escucha y repite cada palabra en voz alta. Los sustantivos llevan siempre el/la: apréndelos juntos.')}</p>
        <div class="vocab-grid">${u.vocab.map(v => `<div class="vocab-card"><div class="vocab-top">${wordHTML(v)}${speakBtn(v.es)}</div><span class="translation">${esc(v.en)}</span>${v.ex ? `<p class="vocab-ex" lang="es">${esc(v.ex)} ${speakBtn(v.ex)}</p>` : ''}</div>`).join('')}</div></section>`;
      case 'grammar': { const g = u.grammar; return `<section class="card"><span class="tag">${L('Grammar hack', 'Truco gramatical')}</span><h2 class="phrase">${esc(g.title)}</h2>
        <div class="hack">${ico('lightbulb')}<p>${esc(g.hack)}</p></div>${g.tables.map(table).join('')}
        <h3 class="mini-title">${L('Examples', 'Ejemplos')}</h3><div class="examples">${g.examples.map(([es, en]) => `<div class="example">${speakBtn(es)}<div><b lang="es">${esc(es)}</b><em>${esc(en)}</em></div></div>`).join('')}</div>
        <div class="coach-tip"><b>${L('Tip:', 'Consejo:')}</b> ${esc(g.tip)}</div></section>`; }
      case 'dialogue': return `<section class="card"><div class="card-head"><h2 class="card-title">${ico('waveform')}${L('Dialogue', 'Diálogo')}</h2><div class="btn-row"><button type="button" class="secondary-btn" data-action="play-dialogue">${ico('play')}${L('Play dialogue', 'Escuchar diálogo')}</button><button type="button" class="secondary-btn" data-action="toggle-en">${ico('eye')}${ls.showEn ? L('Hide English', 'Ocultar inglés') : L('Show English', 'Mostrar inglés')}</button></div></div>
        <p class="card-sub">${L('Trick 3: read and listen in Spanish first. Reveal the English only for lines you really don\'t get.', 'Truco 3: primero lee y escucha en español. Muestra el inglés solo en las frases que de verdad no entiendas.')}</p>
        <div class="dialogue">${u.dialogue.map(([who, es, en], i) => `<div class="dialogue-row ${who === 'Tú' ? 'me' : ''}" data-line="${i}"><span class="speaker-tag ${who === 'Tú' ? '' : 'waiter'}">${esc(who)}</span><p><b lang="es">${esc(es)}</b>${speakBtn(es)}<em class="${ls.showEn || ls.reveal[i] ? '' : 'hidden-en'}">${ls.showEn || ls.reveal[i] ? esc(en) : `<button type="button" class="text-link" data-action="reveal-line" data-i="${i}">${L('show English', 'ver inglés')}</button>`}</em></p></div>`).join('')}</div></section>`;
      case 'practice': return `<div id="lessonSession"></div>`;
      case 'speak': return `<section class="card"><h2 class="card-title">${ico('mic')}${L('Speak from day one', 'Habla desde el día uno')}</h2><p class="card-sub">${L('Listen, then say each phrase out loud 2–3 times. Exaggerate the R and keep the vowels short.', 'Escucha y di cada frase en voz alta 2 o 3 veces. Exagera la R y mantén las vocales cortas.')}</p>
        ${u.speak.map(p => LQ.ui.shadow(p)).join('')}
        <div class="bottom-buttons"><button type="button" class="primary-btn" data-action="finish-lesson">${ico('check', 'white')}${L('Finish lesson', 'Terminar la lección')}</button></div></section>`;
      case 'done': {
        const r = ls.result, next = C.units[u.n] || null;
        return LQ.ex.summary(r, {
          title: L(`Lesson ${u.n} complete!`, `¡Lección ${u.n} completada!`),
          text: L(`${ls.newCards} words were added to your spaced-repetition deck. You'll review them starting tomorrow.`, `Se añadieron ${ls.newCards} palabras a tu mazo de repaso. Las repasarás desde mañana.`),
          actions: `<button type="button" class="primary-btn" data-go="today">${ico('calendar', 'white')}${L("Back to today's plan", 'Volver al plan de hoy')}</button><button type="button" class="secondary-btn" data-action="lesson-step" data-step="practice">${ico('refresh')}${L('Practice again', 'Practicar otra vez')}</button>${next ? `<button type="button" class="secondary-btn" data-go="lesson/${next.id}">${L('Next lesson', 'Siguiente lección')} ${ico('chevron-right')}</button>` : ''}`
        });
      }
    }
  }
  function navButtons() {
    const i = STEPS.indexOf(ls.step);
    if (['practice', 'speak', 'done'].includes(ls.step)) return '';
    return `<div class="bottom-buttons">${i > 0 ? `<button type="button" class="secondary-btn left-button" data-action="lesson-step" data-step="${STEPS[i - 1]}">${ico('chevron-left')}${L('Back', 'Atrás')}</button>` : ''}<button type="button" class="primary-btn" data-action="lesson-step" data-step="${STEPS[i + 1]}">${L('Continue', 'Continuar')} ${ico('chevron-right', 'white')}</button></div>`;
  }

  V.lesson = (id, ctx) => {
    const u = C.unitById[id];
    if (!u) return LQ.ui.emptyState('book', L('Lesson not found', 'Lección no encontrada'), '', `<button type="button" class="primary-btn" data-go="lessons">${L('All lessons', 'Todas las lecciones')}</button>`);
    if (!ls || ls.id !== id || (ctx && ctx.fresh)) ls = { id, step: 'vocab', showEn: false, reveal: {}, xp0: P.act(LQ.state, U.today()).xp };
    const S = LQ.state, r = S.units[id], phase = C.phases[u.phase - 1], idx = STEPS.indexOf(ls.step);
    const goals = [['vocab', L(`Learn ${u.vocab.length} words`, `Aprender ${u.vocab.length} palabras`)], ['grammar', L('Understand the grammar hack', 'Entender el truco gramatical')], ['dialogue', L('Follow a real dialogue', 'Seguir un diálogo real')], ['practice', L('Practice with exercises', 'Practicar con ejercicios')], ['speak', L('Say it out loud', 'Decirlo en voz alta')]];
    return {
      html: `<section class="hero lesson-hero"><div class="hero-copy"><div class="breadcrumb"><button type="button" class="text-link" data-go="lessons">${L('Lessons', 'Lecciones')}</button> › ${L('Phase', 'Fase')} ${u.phase} · ${esc(L(phase.en, phase.es))}</div>
        <h1><span class="flag" aria-hidden="true"><img src="assets/icons/spanish-flag.svg" alt=""></span> ${L(`Lesson ${u.n}`, `Lección ${u.n}`)}: ${esc(u.title)}</h1><p class="sub">${esc(u.canDo)} <span class="trick-chip">${L('Trick', 'Truco')} ${u.trick} · ${esc(L(...TRICKS[u.trick]))}</span></p></div>
        <img class="hero-art" src="assets/illustrations/${u.art === 'cafe' ? 'cafe-scene' : 'spanish-city'}.svg" alt="" aria-hidden="true"></section>
      <div class="lesson-meta"><b>${L(`Step ${Math.min(idx + 1, 5)} of 5`, `Paso ${Math.min(idx + 1, 5)} de 5`)} · ${esc(L(...STEP_LABEL[ls.step]))}</b><div class="steps" role="progressbar" aria-valuenow="${idx}" aria-valuemin="0" aria-valuemax="5">${STEPS.slice(0, 5).map((s, i) => `<span class="${i < idx ? 'on' : i === idx ? 'now' : ''}"></span>`).join('')}</div>${ico('clock')}<span>~20 min</span>${r ? `<span class="state-badge done">✓ ${L('completed', 'completada')} · ${L('best', 'mejor')} ${Math.round(r.best * 100)}%</span>` : ''}</div>
      <div class="grid lesson-grid"><div class="stack">${stepBody(u)}${navButtons()}</div>
        <aside class="stack"><section class="card"><h2 class="card-title">${ico('target')}${L('Lesson goals', 'Objetivos')}</h2><ul class="goal-list">${goals.map(([s, g]) => `<li><span class="status ${STEPS.indexOf(s) < idx ? 'ok' : ''}">${STEPS.indexOf(s) < idx ? '✓' : ''}</span>${esc(g)}</li>`).join('')}</ul></section>
          <section class="card"><h2 class="card-title">${ico('book')}${L('Vocabulary in this lesson', 'Vocabulario de la lección')}</h2>${u.vocab.slice(0, 6).map(wordRow).join('')}</section>
          <section class="card"><h2 class="card-title">${ico('sparkles')}${L('Your Spanish coach', 'Tu coach de español')}</h2><div class="coach-tip">${esc(u.coach)}</div></section></aside></div>`,
      mount(el) {
        if (ls.step !== 'practice') return;
        const box = el.querySelector('#lessonSession');
        ls.session = LQ.ex.run(box, { items: LQ.ex.fromUnit(u), onFinish(res) { ls.practice = res; ls.step = 'speak'; LQ.app.render(); }, onQuit() { ls.step = 'dialogue'; LQ.app.render(); } });
        return () => ls && ls.session && ls.session.stop();
      }
    };
  };

  // Lesson actions are registered with the app's delegated click handler.
  LQ.actions = LQ.actions || {};
  Object.assign(LQ.actions, {
    'lesson-step'(b) { ls.step = b.dataset.step; LQ.app.render(); document.getElementById('mainContent').scrollTop = 0; },
    'toggle-en'() { ls.showEn = !ls.showEn; LQ.app.render(); },
    'reveal-line'(b) { ls.reveal[b.dataset.i] = true; LQ.app.render(); },
    'play-all'() { const u = C.unitById[ls.id]; LQ.speech.speak(u.vocab.map(v => v.es).join('. ') + '.'); },
    'play-dialogue'() {
      const u = C.unitById[ls.id]; let i = 0;
      const rows = () => document.querySelectorAll('.dialogue-row');
      const step = () => {
        rows().forEach(r => r.classList.toggle('playing', +r.dataset.line === i));
        if (i >= u.dialogue.length) return;
        LQ.speech.speak(u.dialogue[i][1], { onend() { i++; setTimeout(step, 350); } });
      };
      step();
    },
    'finish-lesson'() {
      const S = LQ.state, today = U.today(), u = C.unitById[ls.id], before = Object.keys(S.cards).length;
      const score = ls.practice ? ls.practice.score : 1;
      P.act(S, today).speak += u.speak.length;
      LQ.app.handle(P.learnUnit(S, u, score, today));
      ls.newCards = Object.keys(S.cards).length - before;
      ls.result = { ...(ls.practice || { answered: 0, first: 0, selfRated: 0 }), score, xp: P.act(S, today).xp - ls.xp0, selfRated: u.speak.length };
      ls.step = 'done'; LQ.app.save(); LQ.app.render(); LQ.ui.flushCelebrations();
    }
  });
})(typeof window !== 'undefined' ? window : globalThis);
