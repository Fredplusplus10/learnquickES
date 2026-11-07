/* Immersion: listening log (trick 3), device missions (trick 2) and language exchanges (trick 4). */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, CAL = LQ.calendar;
  const { ico, esc, speakBtn, meter } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  LQ.actions = LQ.actions || {}; LQ.changes = LQ.changes || {}; LQ.submits = LQ.submits || {};

  // Dreaming Spanish measures progress in hours of comprehensible input; their published level thresholds.
  const DS_LEVELS = [0, 50, 150, 300, 600, 1000, 1500];
  const KINDS = { podcast: ['Podcast', 'Pódcast'], video: ['Video', 'Video'], series: ['Series / movie', 'Serie / película'], reading: ['Reading', 'Lectura'], music: ['Music (with lyrics)', 'Música (con letra)'], other: ['Other', 'Otro'] };

  V.immersion = () => {
    const S = LQ.state, today = U.today(), T = P.totals(S, today), a = S.activity[today] || {}, target = CAL.INPUT_TARGET[S.plan.days];
    const todayMin = (a.input || 0) + (a.island || 0) + (a.talk || 0), hours = T.inputMin / 60;
    const lvl = DS_LEVELS.filter(h => hours >= h).length, next = DS_LEVELS[lvl];
    const missionsDone = C.missions.filter(m => S.missions[m.id]).length;
    const seriesLevel = LQ.progress.seriesLevel(S);
    return `<section class="page-head"><h1>${L('Immersion', 'Inmersión')}</h1><p class="sub">${L('The app teaches the system; immersion builds the ear and the habit. Log your listening, switch your devices to Spanish and talk to real people.', 'La app enseña el sistema; la inmersión entrena el oído y el hábito. Registra lo que escuchas, pon tus dispositivos en español y habla con personas reales.')}</p></section>
    <section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('headphones')}${L('Log your Spanish input', 'Registra tu español')}</h2>
        <p class="metric-row"><b>${Math.round(todayMin)} / ${target} min</b> <small>${L('today (listening + island + exchanges)', 'hoy (escucha + isla + intercambios)')}</small></p>${meter(todayMin / target * 100, L('Immersion today', 'Inmersión hoy'))}
        <form class="log-form" data-submit="log-input">
          <label>${L('Minutes', 'Minutos')}<input name="min" type="number" min="1" max="600" value="20" required></label>
          <label>${L('Type', 'Tipo')}<select name="kind">${Object.entries(KINDS).map(([k, v]) => `<option value="${k}">${esc(L(...v))}</option>`).join('')}</select></label>
          <label class="grow">${L('What did you listen to?', '¿Qué escuchaste?')}<input name="source" list="sourceList" maxlength="80" placeholder="No Hay Tos, Friends…"></label>
          <datalist id="sourceList">${C.resources.map(r => `<option value="${esc(r.name)}">`).join('')}</datalist>
          <button type="submit" class="primary-btn">${ico('check', 'white')}${L('Log', 'Registrar')}</button></form>
        ${S.immersion.length ? `<ul class="log-list">${S.immersion.slice(-6).reverse().map(e => `<li><span>${esc(U.fmtDate(e.date, LQ.lang(), { month: 'short', day: 'numeric' }))}</span><b>${e.min} min</b><span>${esc(L(...(KINDS[e.kind] || KINDS.other)))}${e.source ? ' · ' + esc(e.source) : ''}</span><button type="button" class="icon-btn" data-action="del-input" data-id="${e.id}" aria-label="${L('Delete entry', 'Borrar registro')}">${ico('x')}</button></li>`).join('')}</ul>` : ''}</div>
      <div class="card"><h2 class="card-title">${ico('chart')}${L('Listening hours', 'Horas de escucha')}</h2>
        <div class="metric-big">${hours.toFixed(1)} h</div>
        <p class="card-sub">${next ? L(`Dreaming Spanish's roadmap measures progress in hours of comprehensible input. Next milestone: ${next} h (their level ${lvl + 1}).`, `La hoja de ruta de Dreaming Spanish mide el progreso en horas de input comprensible. Próxima meta: ${next} h (su nivel ${lvl + 1}).`) : L('You passed the last Dreaming Spanish milestone (1,500 h).', 'Superaste la última meta de Dreaming Spanish (1.500 h).')}</p>
        ${next ? meter((hours - DS_LEVELS[lvl - 1]) / (next - DS_LEVELS[lvl - 1]) * 100, L('Progress to next milestone', 'Progreso a la próxima meta')) : ''}
        <h3 class="mini-title">${L('Series without English', 'Series sin inglés')}</h3><ul class="idea-list">${C.seriesTips.map(t => `<li>${esc(t)}</li>`).join('')}</ul></div>
    </section>
    <section class="card"><h2 class="card-title">${ico('headphones')}${L('Recommended input (A2 → B1)', 'Input recomendado (A2 → B1)')}</h2><p class="card-sub">${L('Choose content you understand at least 70–80% of. Use 0.9× speed if it feels too fast.', 'Elige contenido que entiendas al menos en un 70–80 %. Usa velocidad 0,9× si va muy rápido.')}</p>
      <div class="resource-grid">${C.resources.map(r => `<a class="resource" href="${esc(r.url)}" target="_blank" rel="noopener noreferrer"><div class="resource-top"><b>${esc(r.name)}</b>${ico('external')}</div><div class="tags"><span class="tag">${esc(r.level)}</span><span class="tag soft">${esc(r.kind)}</span><span class="tag soft">${esc(r.accent)}</span></div><p>${esc(r.note)}</p></a>`).join('')}</div></section>
    <section class="card"><div class="card-head"><h2 class="card-title">${ico('play')}${L('Series in Spanish', 'Series en español')}</h2><span class="tag">${L('Your level', 'Tu nivel')}: ${seriesLevel}</span></div>
      <p class="card-sub">${seriesLevel === 'B1' ? L('You passed the phase 4 checkpoint, so B1 series are yours.', 'Aprobaste el control de la fase 4: las series de B1 son para ti.') : seriesLevel === 'A2' ? L('Pass the phase 4 checkpoint with 70% and B1 series open up.', 'Aprueba el control de la fase 4 con un 70 % y se abren las series de B1.') : L('Pass the phase 2 checkpoint with 70% and A2 series open up.', 'Aprueba el control de la fase 2 con un 70 % y se abren las series de A2.')}</p>
      ${['A1', 'A2', 'B1'].map(lv => { const list = C.series.filter(x => x.level === lv); return `<h3 class="mini-title">${lv}${lv === seriesLevel ? ' · ' + L('your level', 'tu nivel') : ''}</h3><div class="resource-grid">${list.map(x => `<div class="resource"><div class="resource-top"><b>${esc(x.name)}</b></div><div class="tags"><span class="tag">${esc(x.level)}</span><span class="tag soft">${esc(x.kind)}</span><span class="tag soft">${esc(x.years)}</span></div><p>${esc(x.note)}</p><div class="tags">${x.where.map(([service, detail]) => `<span class="tag soft">${esc(service)}${detail ? ' · ' + esc(detail) : ''}</span>`).join('')}</div></div>`).join('')}</div>`; }).join('')}
      <p class="card-sub">${esc(C.seriesWhere)} ${L("All of these are from Spain, with the Castilian accent and vosotros, which the course doesn't teach: good extra listening, not a model to copy.", 'Todas son de España, con acento castellano y vosotros, que el curso no enseña: buena escucha extra, no un modelo que copiar.')}</p></section>
    <section class="grid two-grid">
      <div class="card"><div class="card-head"><h2 class="card-title">${ico('globe')}${L('Goodbye English: devices', 'Adiós al inglés: dispositivos')}</h2><span class="tag">${missionsDone}/${C.missions.length}</span></div>
        <p class="card-sub">${L('Trick 2: every notification and menu becomes a tiny Spanish lesson. Tick each one when it is done.', 'Truco 2: cada notificación y cada menú se vuelve una mini lección. Marca cada una cuando la hagas.')}</p>
        <div class="mission-list">${C.missions.map(m => `<label class="mission ${S.missions[m.id] ? 'done' : ''}"><input type="checkbox" data-change="mission" value="${m.id}" ${S.missions[m.id] ? 'checked' : ''}><div><b>${esc(m.title)}</b><small>${esc(m.how)}</small>${m.id === 'app' ? `<button type="button" class="text-link" data-action="ui-lang" data-v="${S.profile.uiLang === 'es' ? 'en' : 'es'}">${S.profile.uiLang === 'es' ? 'Volver la interfaz a inglés' : 'Switch the interface to Spanish'}</button>` : ''}</div></label>`).join('')}</div></div>
      <div class="card"><h2 class="card-title">${ico('users')}${L('Language exchanges', 'Intercambios de idiomas')}</h2>
        <p class="card-sub">${L('Trick 4: on Tandem or HelloTalk, Spanish speakers practise with you in exchange for English. Aim for 1–2 sessions a week from phase 2.', 'Truco 4: en Tandem o HelloTalk, hispanohablantes practican contigo a cambio de inglés. Apunta a 1 o 2 sesiones por semana desde la fase 2.')}</p>
        <div class="btn-row"><a class="secondary-btn" href="https://www.tandem.net" target="_blank" rel="noopener noreferrer">Tandem ${ico('external')}</a><a class="secondary-btn" href="https://www.hellotalk.com" target="_blank" rel="noopener noreferrer">HelloTalk ${ico('external')}</a></div>
        <form class="log-form" data-submit="log-exchange">
          <label>${L('Minutes', 'Minutos')}<input name="min" type="number" min="5" max="300" value="30" required></label>
          <label class="grow">${L('Partner (optional)', 'Compañero (opcional)')}<input name="partner" maxlength="60"></label>
          <label class="grow">${L('What did you talk about?', '¿De qué hablaron?')}<input name="note" maxlength="140"></label>
          <button type="submit" class="primary-btn">${ico('check', 'white')}${L('Log', 'Registrar')}</button></form>
        ${S.exchanges.length ? `<ul class="log-list">${S.exchanges.slice(-4).reverse().map(e => `<li><span>${esc(U.fmtDate(e.date, LQ.lang(), { month: 'short', day: 'numeric' }))}</span><b>${e.min} min</b><span>${esc(e.partner || '')}${e.note ? ' · ' + esc(e.note) : ''}</span></li>`).join('')}</ul>` : ''}
        <h3 class="mini-title">${L('Phrases for your first exchange', 'Frases para tu primer intercambio')}</h3>
        ${C.exchangePhrases.map(([es, en]) => `<div class="example">${speakBtn(es)}<div><b lang="es">${esc(es)}</b><em>${esc(en)}</em></div></div>`).join('')}
        <div class="coach-tip"><b>${L('Stay safe:', 'Seguridad:')}</b> ${L("keep chats inside the app at first, don't share your address, workplace or financial details, and leave any conversation that makes you uncomfortable.", 'al principio, mantén las conversaciones dentro de la app, no compartas tu dirección, tu lugar de trabajo ni datos financieros, y deja cualquier conversación que te incomode.')}</div></div>
    </section>`;
  };

  function logInput(min, kind, source) {
    const S = LQ.state, today = U.today();
    S.immersion.push({ id: U.uid(), date: today, min, kind, source });
    LQ.app.handle(P.logMinutes(S, 'input', min, today)); LQ.app.save(); LQ.app.render(); LQ.ui.flushCelebrations();
    LQ.ui.toast(L(`+${min} min of Spanish logged.`, `+${min} min de español registrados.`));
  }
  Object.assign(LQ.submits, {
    'log-input'(f) { const min = U.clamp(Math.round(+f.min.value), 1, 600); if (min) logInput(min, f.kind.value, f.source.value.trim()); },
    'log-exchange'(f) {
      const S = LQ.state, today = U.today(), min = U.clamp(Math.round(+f.min.value), 1, 300);
      S.exchanges.push({ id: U.uid(), date: today, min, partner: f.partner.value.trim(), note: f.note.value.trim() });
      LQ.app.handle(P.logMinutes(S, 'talk', min, today)); LQ.app.save(); LQ.app.render(); LQ.ui.flushCelebrations();
      LQ.ui.toast(L('Exchange logged. ¡Bien hecho!', 'Intercambio registrado. ¡Bien hecho!'));
    }
  });
  Object.assign(LQ.actions, {
    'log-input'(b) { logInput(+b.dataset.min, 'other', ''); },
    'del-input'(b) {
      const S = LQ.state, i = S.immersion.findIndex(e => e.id === b.dataset.id); if (i < 0) return;
      const [e] = S.immersion.splice(i, 1), a = S.activity[e.date];
      // Viewing imported in the desktop app counted no XP, so none is taken back.
      if (a) { a.input = Math.max(0, a.input - e.min); if (!e.imported) a.xp = Math.max(0, a.xp - e.min); }
      LQ.app.save(); LQ.app.render();
    },
    'ui-lang'(b) { LQ.state.profile.uiLang = b.dataset.v; if (b.dataset.v === 'es') LQ.state.missions.app = LQ.state.missions.app || U.today(); LQ.app.handle(P.checkAchievements(LQ.state, U.today())); LQ.app.save(); LQ.app.render(); }
  });
  LQ.changes.mission = el => {
    const S = LQ.state;
    if (el.checked) S.missions[el.value] = U.today(); else delete S.missions[el.value];
    LQ.app.handle(P.checkAchievements(S, U.today())); LQ.app.save(); LQ.app.render();
  };
})(typeof window !== 'undefined' ? window : globalThis);
