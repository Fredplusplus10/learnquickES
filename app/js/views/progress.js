/* Progress: roadmap, measured totals, minutes per day, accuracy by skill, streak calendar, achievements and
   B1 readiness signals. Every number comes from recorded activity — nothing is a sample value. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, CAL = LQ.calendar;
  const { ico, esc, meter } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};

  const SKILLS = [['verbs', ['Verb forms', 'Conjugación']], ['past', ['Fui vs. iba', 'Fui vs. iba']], ['grammar', ['Grammar', 'Gramática']], ['vocab', ['Vocabulary', 'Vocabulario']], ['listening', ['Listening', 'Comprensión auditiva']], ['gender', ['El / la', 'El / la']], ['cognates', ['Cognates', 'Cognados']], ['falsefriends', ['False friends', 'Falsos amigos']], ['review', ['Flashcards', 'Tarjetas']]];

  function roadmap(S, st) {
    return `<div class="roadmap-grid">${C.phases.map(ph => {
      const units = C.units.filter(u => u.phase === ph.n), done = units.filter(u => S.units[u.id]).length, cp = S.checkpoints[ph.n];
      const state = cp ? 'done' : (!st.finished && st.entry.phase === ph.n) ? 'current' : '';
      return `<div class="milestone ${state}">${ico(ph.icon)}<h3>${L('Phase', 'Fase')} ${ph.n} · ${esc(L(ph.en, ph.es))}</h3><p>${esc(ph.desc)}</p>
        <span class="state-badge ${state}">${cp ? '✓ ' + L('Complete', 'Completada') : state === 'current' ? L('In progress', 'En curso') : L('Upcoming', 'Próxima')}</span>
        <small>${L(`${done}/5 lessons`, `${done}/5 lecciones`)}${cp ? ` · ${L('checkpoint', 'control')} ${Math.round(cp.best * 100)}%` : ''}</small></div>`;
    }).join('')}</div>`;
  }
  // Single-series bar chart (minutes in Spanish per day). Values are labelled on hover/focus and listed in a table.
  function minutesChart(S, today) {
    const days = Array.from({ length: 14 }, (_, i) => U.addDays(today, i - 13));
    const vals = days.map(d => { const a = S.activity[d]; return a ? Math.round(a.sec / 60 + a.input + a.island + (a.talk || 0)) : 0; });
    const max = Math.max(30, ...vals), lang = LQ.lang();
    const label = d => U.fmtDate(d, lang, { weekday: 'short', month: 'short', day: 'numeric' });
    return `<div class="bars chart-bars" role="img" aria-label="${L('Minutes in Spanish per day, last 14 days', 'Minutos en español por día, últimos 14 días')}">${days.map((d, i) => `<div tabindex="0" style="--h:${Math.max(vals[i] ? 3 : 0, vals[i] / max * 100)}%" data-label="${U.parse(d).getDate()}" data-tip="${esc(label(d))}: ${vals[i]} min" class="${d === today ? 'is-today' : ''}"></div>`).join('')}</div>
      <details class="table-toggle"><summary>${L('Show as table', 'Ver como tabla')}</summary><table class="g-table"><tbody>${days.map((d, i) => `<tr><td>${esc(label(d))}</td><td>${vals[i]} min</td></tr>`).join('')}</tbody></table></details>`;
  }
  // Last five weeks (Mon–Sun rows) of streak states; each state has its own symbol, so colour is never the only signal.
  function recentCalendar(S, today, si) {
    const first = U.addDays(U.weekStart(today), -28), cells = [];
    for (let i = 0; i < 35; i++) {
      const d = U.addDays(first, i), sched = CAL.isStudyDay(d, S.plan);
      const state = si.active.has(d) ? 'done' : si.frozen.has(d) ? 'frozen' : d > today ? 'future' : d === today ? 'today' : sched && d >= S.plan.start ? 'missed' : 'off';
      const mark = state === 'done' ? '✓' : state === 'frozen' ? '❄' : state === 'missed' ? '·' : '';
      cells.push(`<span class="cal-day ${state}" title="${esc(U.fmtDate(d, LQ.lang()))}"><small>${U.parse(d).getDate() === 1 ? esc(U.fmtDate(d, LQ.lang(), { month: 'short' })) : U.parse(d).getDate()}</small>${mark}</span>`);
    }
    const dow = LQ.lang() === 'es' ? ['L', 'M', 'X', 'J', 'V', 'S', 'D'] : ['M', 'T', 'W', 'T', 'F', 'S', 'S'];
    return `<div class="cal-grid">${dow.map(d => `<b>${d}</b>`).join('')}${cells.join('')}</div>
      <p class="legend"><span class="cal-key done">✓</span> ${L('studied', 'estudiado')} <span class="cal-key frozen">❄</span> ${L('freeze', 'protegido')} <span class="cal-key missed">·</span> ${L('missed', 'perdido')} <span class="cal-key off"></span> ${L('day off', 'libre')}</p>`;
  }
  function readiness(S, T) {
    const acc = k => T.accuracy(k), ex = k => (T.ex[k] || [0, 0])[1];
    const items = [
      [Object.keys(S.units).length >= 25, L(`All 25 lessons (${Object.keys(S.units).length}/25)`, `Las 25 lecciones (${Object.keys(S.units).length}/25)`)],
      [ex('past') >= 30 && acc('past') >= 0.85, L(`Fui vs. iba: 85%+ over 30+ answers (${acc('past') === null ? '—' : Math.round(acc('past') * 100) + '%'})`, `Fui vs. iba: 85 %+ en 30+ respuestas (${acc('past') === null ? '—' : Math.round(acc('past') * 100) + ' %'})`)],
      [ex('verbs') >= 100 && acc('verbs') >= 0.8, L(`Verb forms: 80%+ over 100+ answers (${acc('verbs') === null ? '—' : Math.round(acc('verbs') * 100) + '%'})`, `Verbos: 80 %+ en 100+ respuestas (${acc('verbs') === null ? '—' : Math.round(acc('verbs') * 100) + ' %'})`)],
      [T.words.remembered >= 200, L(`200+ words remembered (${T.words.remembered})`, `200+ palabras recordadas (${T.words.remembered})`)],
      [T.inputMin >= 3000, L(`50+ hours of listening (${(T.inputMin / 60).toFixed(1)} h)`, `50+ horas de escucha (${(T.inputMin / 60).toFixed(1)} h)`)],
      [S.exchanges.length >= 5, L(`5+ real conversations (${S.exchanges.length})`, `5+ conversaciones reales (${S.exchanges.length})`)],
      [(S.checkpoints[5] || {}).best >= 0.7, L('B1 mock exam at 70%+', 'Simulacro B1 con 70 %+')]
    ];
    return `<ul class="goal-list readiness">${items.map(([ok, t]) => `<li><span class="status ${ok ? 'ok' : ''}">${ok ? '✓' : ''}</span>${esc(t)}</li>`).join('')}</ul>`;
  }

  V.progress = () => {
    const S = LQ.state, today = U.today(), st = P.planStatus(S), T = P.totals(S, today), si = P.streakInfo(S, today), pace = P.pace(S, today);
    const lessons = Object.keys(S.units).length;
    return `<section class="page-head"><h1>${L('Progress', 'Progreso')}</h1><p class="sub">${L(`Day ${Math.min(st.doneCount + 1, st.total)} of your ${st.total}-day plan. Measured from your own sessions.`, `Día ${Math.min(st.doneCount + 1, st.total)} de tu plan de ${st.total} días. Medido en tus propias sesiones.`)}</p></section>
    <section class="card roadmap"><h2 class="card-title">${ico('route')}${L('Your roadmap to B1', 'Tu camino al B1')}</h2>${roadmap(S, st)}</section>
    <section class="summary-row">
      <div class="card summary-tile"><small>${L('Time in Spanish', 'Tiempo en español')}</small><b>${(T.spanishMin / 60).toFixed(1)} h</b><span>${L(`app ${(T.appMin / 60).toFixed(1)} · listening ${(T.inputMin / 60).toFixed(1)} · island ${(T.islandMin / 60).toFixed(1)} · talk ${(T.talkMin / 60).toFixed(1)}`, `app ${(T.appMin / 60).toFixed(1)} · escucha ${(T.inputMin / 60).toFixed(1)} · isla ${(T.islandMin / 60).toFixed(1)} · charla ${(T.talkMin / 60).toFixed(1)}`)}</span></div>
      <div class="card summary-tile"><small>${L('Lessons', 'Lecciones')}</small><b>${lessons} / 25</b><span>${L(`${st.doneCount} of ${st.total} plan days done`, `${st.doneCount} de ${st.total} días del plan`)}</span></div>
      <div class="card summary-tile"><small>${L('Words', 'Palabras')}</small><b>${T.words.remembered} / ${T.words.seen}</b><span>${L('remembered / in your deck', 'recordadas / en tu mazo')}</span></div>
      <div class="card summary-tile"><small>${L('Streak', 'Racha')}</small><b>${si.count} ${L(si.count === 1 ? 'day' : 'days', si.count === 1 ? 'día' : 'días')}</b><span>${L(`best ${si.longest} · ${si.freezes} freezes`, `mejor ${si.longest} · ${si.freezes} protectores`)}</span></div>
    </section>
    <section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('chart')}${L('Minutes in Spanish per day', 'Minutos en español por día')}</h2><p class="card-sub">${L('Last 14 days · app time + listening + island + exchanges. Hover a bar for its value.', 'Últimos 14 días · app + escucha + isla + intercambios. Pasa el ratón sobre una barra.')}</p>${minutesChart(S, today)}</div>
      <div class="card"><h2 class="card-title">${ico('target')}${L('Accuracy by skill', 'Acierto por habilidad')}</h2><p class="card-sub">${L('First-try answers, all time.', 'Respuestas al primer intento, desde el principio.')}</p>
        ${SKILLS.map(([k, lab]) => { const e = T.ex[k], a = T.accuracy(k); return `<div class="skill-row"><span>${esc(L(...lab))}</span>${a === null ? `<small class="nodata">${L('no data yet', 'sin datos')}</small>` : `${meter(a * 100, L(...lab))}<b>${Math.round(a * 100)}%</b><small>${e[1]}</small>`}</div>`; }).join('')}</div>
    </section>
    <section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('flame')}${L('Last 5 weeks', 'Últimas 5 semanas')}</h2><p class="card-sub">${L(`Pace: ${pace.diff >= 0 ? 'on track' : -pace.diff + (pace.diff === -1 ? ' day' : ' days') + ' behind'}${pace.target ? ' · target ' + U.fmtDate(pace.target, 'en') : ''}.`, `Ritmo: ${pace.diff >= 0 ? 'al día' : -pace.diff + (pace.diff === -1 ? ' día' : ' días') + ' de retraso'}${pace.target ? ' · fecha objetivo ' + U.fmtDate(pace.target, 'es') : ''}.`)}</p>${recentCalendar(S, today, si)}</div>
      <div class="card"><h2 class="card-title">${ico('award')}${L('B1 readiness signals', 'Señales de B1')}</h2><p class="card-sub">${L('Concrete signals that you are close to a functional B1. They are guides, not a certificate.', 'Señales concretas de que te acercas a un B1 funcional. Son una guía, no un certificado.')}</p>${readiness(S, T)}</div>
    </section>
    <section class="card"><h2 class="card-title">${ico('star')}${L('Achievements', 'Logros')}</h2><p class="card-sub">${L(`${Object.keys(S.achievements).length} of ${P.ACHIEVEMENTS.length} earned.`, `${Object.keys(S.achievements).length} de ${P.ACHIEVEMENTS.length} conseguidos.`)}</p>
      <div class="achievement-grid">${P.ACHIEVEMENTS.map(a => { const got = S.achievements[a.id]; return `<div class="achievement ${got ? '' : 'locked'}"><div class="medal">${ico(got ? a.icon : 'lock')}</div><b>${esc(L(a.en, a.es))}</b><small>${got ? esc(U.fmtDate(got, LQ.lang())) : L('locked', 'bloqueado')}</small></div>`; }).join('')}</div></section>`;
  };
})(typeof window !== 'undefined' ? window : globalThis);
