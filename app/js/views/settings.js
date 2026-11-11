/* Settings: plan, schedule, voice, interface, reminders and data (backup / restore / reset). */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, CAL = LQ.calendar;
  const { ico, esc } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};
  LQ.actions = LQ.actions || {}; LQ.changes = LQ.changes || {}; LQ.inputs = LQ.inputs || {};

  const ACCENTS = [['es-MX', ['Mexico (Latin America)', 'México (Latinoamérica)']], ['es-US', ['US Spanish', 'Español de EE. UU.']], ['es-ES', ['Spain', 'España']]];
  const DOW = () => LQ.lang() === 'es' ? ['Dom', 'Lun', 'Mar', 'Mié', 'Jue', 'Vie', 'Sáb'] : ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
  // Average in-app minutes per study day for a plan length (used by Settings and onboarding).
  LQ.planMinutes = days => Math.round(P.schedule(days).reduce((s, e) => s + CAL.dayMinutes(CAL.dayTasks(e, days)), 0) / days);

  function voiceOptions(S) {
    const voices = LQ.speech.voices();
    if (!voices.length) return `<option value="">${L('No Spanish voices found', 'No hay voces en español')}</option>`;
    return `<option value="">${L('Automatic (best for the accent)', 'Automática (la mejor para el acento)')}</option>` + voices.map(v => `<option value="${esc(v.voiceURI)}" ${v.voiceURI === S.profile.voiceURI ? 'selected' : ''}>${esc(v.name)} · ${esc(v.lang)}</option>`).join('');
  }
  LQ.voiceBlock = S => `<div class="setting-option"><div><b>${L('Accent', 'Acento')}</b><small>${L('Neutral Latin-American Spanish is the default — closest to what you will hear in the US.', 'El español latinoamericano neutro es el predeterminado: el más cercano a lo que oirás en EE. UU.')}</small></div><select data-change="accent">${ACCENTS.map(([k, n]) => `<option value="${k}" ${S.profile.accent === k ? 'selected' : ''}>${esc(L(...n))}</option>`).join('')}</select></div>
    <div class="setting-option"><div><b>${L('Voice', 'Voz')}</b><small>${L('Voices installed on this Mac.', 'Voces instaladas en este Mac.')}</small></div><select data-change="voice">${voiceOptions(S)}</select></div>
    <div class="setting-option"><div><b>${L('Speed', 'Velocidad')}</b><small>${L('0.9× is a good start for learners.', '0,9× es un buen comienzo.')}</small></div><div class="range"><input type="range" min="0.6" max="1.1" step="0.05" value="${S.profile.rate}" data-change="rate" aria-label="${L('Speech speed', 'Velocidad de la voz')}"><b>${S.profile.rate.toFixed(2)}×</b></div></div>
    <div class="setting-option"><div><b>${L('Test', 'Probar')}</b></div><button type="button" class="secondary-btn" data-speak="Hola, ¿qué tal? Me llamo Lucía y vivo en Guadalajara. ¡Vamos a practicar español juntos!">${ico('speaker')}${L('Play a sample', 'Escuchar ejemplo')}</button></div>
    <div class="coach-tip">${L('<b>Better voices on the Mac:</b> System Settings → Accessibility → Spoken Content → System Voice → Manage Voices… → Spanish → download an "Enhanced" or "Premium" voice (for example Paulina or Mónica). Then reopen LearnQuick.', '<b>Mejores voces en el Mac:</b> Ajustes del Sistema → Accesibilidad → Contenido leído → Voz del sistema → Gestionar voces… → Español → descarga una voz «Mejorada» o «Premium» (por ejemplo Paulina o Mónica). Luego vuelve a abrir LearnQuick.')}</div>`;

  V.settings = () => {
    const S = LQ.state, p = S.plan, today = U.today();
    const holidays = []; for (let d = today; holidays.length < 3 && d < U.addDays(today, 400); d = U.addDays(d, 1)) if (CAL.isHoliday(d)) holidays.push(d);
    return `<section class="page-head"><h1>${L('Settings', 'Ajustes')}</h1><p class="sub">${L('Everything is stored only on this Mac. No account, no tracking.', 'Todo se guarda solo en este Mac. Sin cuenta y sin rastreo.')}</p></section>
    <section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('route')}${L('Study plan', 'Plan de estudio')}</h2>
        <div class="setting-option"><div><b>${L('Plan length', 'Duración del plan')}</b><small>${L('Same 25 lessons; longer plans add practice days.', 'Las mismas 25 lecciones; los planes largos añaden días de práctica.')}</small></div><div class="pill-row">${[30, 60, 90].map(d => `<button type="button" class="pill ${p.days === d ? 'active' : ''}" data-action="plan-days" data-v="${d}">${d} ${L('days', 'días')} · ~${LQ.planMinutes(d)}′</button>`).join('')}</div></div>
        <div class="setting-option"><div><b>${L('Start date', 'Fecha de inicio')}</b><small>${L('Used to measure your pace.', 'Se usa para medir tu ritmo.')}</small></div><input type="date" value="${p.start}" data-change="plan-start"></div>
        <div class="setting-option"><div><b>${L('Study days', 'Días de estudio')}</b><small>${L('Other days never break your streak.', 'Los demás días nunca rompen tu racha.')}</small></div><div class="dow-row">${[1, 2, 3, 4, 5, 6, 0].map(d => `<label class="dow ${p.studyDays.includes(d) ? 'on' : ''}"><input type="checkbox" data-change="study-day" value="${d}" ${p.studyDays.includes(d) ? 'checked' : ''}>${DOW()[d]}</label>`).join('')}</div></div>
        <div class="setting-option"><div><b>${L('Skip US federal holidays', 'Saltar festivos federales de EE. UU.')}</b><small>${L('Next:', 'Próximos:')} ${holidays.map(h => esc(U.fmtDate(h, LQ.lang(), { month: 'short', day: 'numeric' }))).join(', ')}</small></div><input type="checkbox" class="switch" data-change="skip-holidays" ${p.skipHolidays ? 'checked' : ''} aria-label="${L('Skip US federal holidays', 'Saltar festivos')}"></div>
        <div class="setting-option"><div><b>${L('Days off', 'Días libres')}</b><small>${L('Vacation or busy days. They pause the plan without breaking your streak.', 'Vacaciones o días ocupados. Pausan el plan sin romper la racha.')}</small>
          ${p.daysOff.length ? `<div class="chip-row">${p.daysOff.slice().sort().map(d => `<span class="chip">${esc(U.fmtDate(d, LQ.lang()))}<button type="button" data-action="day-off-remove" data-v="${d}" aria-label="${L('Remove', 'Quitar')}">×</button></span>`).join('')}</div>` : ''}</div>
          <div class="inline-form"><input type="date" id="dayOffInput" min="${today}"><button type="button" class="secondary-btn" data-action="day-off-add">${L('Add', 'Añadir')}</button></div></div>
        <div class="setting-option"><div><b>${L('Daily reminder', 'Recordatorio diario')}</b><small>${L('Adds a repeating event to the Calendar app on your study days.', 'Añade un evento repetido en la app Calendario en tus días de estudio.')}</small></div><div class="inline-form"><input type="time" value="${p.reminder}" data-change="reminder"><button type="button" class="secondary-btn" data-action="ics">${ico('calendar')}${L('Add to Calendar', 'Añadir al Calendario')}</button></div></div>
      </div>
      <div class="card"><h2 class="card-title">${ico('speaker')}${L('Voice', 'Voz')}</h2>${LQ.voiceBlock(S)}</div>
    </section>
    <section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('settings')}${L('Interface & practice', 'Interfaz y práctica')}</h2>
        <div class="setting-option"><div><b>${L('Your name', 'Tu nombre')}</b></div><input type="text" id="nameInput" value="${esc(S.profile.name)}" maxlength="30" data-change="name"></div>
        <div class="setting-option"><div><b>${L('Interface language', 'Idioma de la interfaz')}</b><small>${L('Spanish mode = more immersion (trick 2). The language island always switches to Spanish.', 'Modo español = más inmersión (truco 2). La isla lingüística siempre cambia a español.')}</small></div><div class="pill-row"><button type="button" class="pill ${S.profile.uiLang === 'en' ? 'active' : ''}" data-action="ui-lang" data-v="en">English</button><button type="button" class="pill ${S.profile.uiLang === 'es' ? 'active' : ''}" data-action="ui-lang" data-v="es">Español</button></div></div>
        <div class="setting-option"><div><b>${L('Practice vosotros', 'Practicar vosotros')}</b><small>${L('Only used in Spain. Off by default.', 'Solo se usa en España. Desactivado por defecto.')}</small></div><input type="checkbox" class="switch" data-change="vosotros" ${S.profile.vosotros ? 'checked' : ''} aria-label="vosotros"></div>
        <div class="setting-option"><div><b>${L('Cards per review', 'Tarjetas por repaso')}</b></div><select data-change="review-limit">${[10, 20, 30, 50].map(n => `<option value="${n}" ${S.profile.reviewLimit === n ? 'selected' : ''}>${n}</option>`).join('')}</select></div>
      </div>
      <div class="card"><h2 class="card-title">${ico('shield')}${L('Your data', 'Tus datos')}</h2>
        <p class="card-sub">${L('Your progress lives in this browser on this Mac. Download a backup every week — if browser data is cleared, the backup brings everything back.', 'Tu progreso vive en este navegador de este Mac. Descarga una copia cada semana: si se borran los datos del navegador, la copia lo recupera todo.')}</p>
        <div class="setting-option"><div><b>${L('Last backup', 'Última copia')}</b><small>${S.lastBackup ? esc(U.fmtDate(S.lastBackup, LQ.lang())) : L('never', 'nunca')}</small></div><button type="button" class="primary-btn" data-action="backup">${ico('download', 'white')}${L('Download backup', 'Descargar copia')}</button></div>
        <div class="setting-option"><div><b>${L('Restore from a backup', 'Restaurar una copia')}</b><small>${L('Replaces the current progress.', 'Reemplaza el progreso actual.')}</small></div><label class="secondary-btn file-btn">${ico('upload')}${L('Choose file…', 'Elegir archivo…')}<input type="file" accept=".json,application/json" data-change="restore" hidden></label></div>
        <div class="setting-option"><div><b>${L('Start over', 'Empezar de cero')}</b><small>${L('Deletes all progress on this Mac.', 'Borra todo el progreso en este Mac.')}</small></div><button type="button" class="secondary-btn danger" data-action="reset-ask">${ico('trash')}${L('Reset…', 'Reiniciar…')}</button></div>
        ${LQ.store.demo ? `<p class="pill-note">${L('Demo mode: changes here only affect the demo data.', 'Modo demo: los cambios solo afectan a los datos de demostración.')}</p>` : ''}
      </div>
    </section>`;
  };

  // Changing the plan length re-maps progress onto the new schedule: days whose lesson/checkpoint is done, and
  // practice days that come before a completed lesson, count as done. Lessons, words and streak are untouched.
  function remapPlan(S, days) {
    const today = U.today(), sched = P.schedule(days), done = [];
    const unitDone = e => !!S.units[C.units[e.unit].id];
    for (let i = 0; i < sched.length; i++) {
      const e = sched[i];
      const ok = e.type === 'lesson' ? unitDone(e) : e.type === 'checkpoint' ? !!S.checkpoints[e.phase] : sched.slice(i + 1).some(x => x.phase === e.phase && ((x.type === 'lesson' && unitDone(x)) || (x.type === 'checkpoint' && S.checkpoints[x.phase])));
      if (!ok) break;
      done.push({ n: e.n, date: today });
    }
    S.plan.days = days; S.done = done; S.tasks = {};
  }
  function icsFile(S) {
    const p = S.plan, [hh, mm] = p.reminder.split(':'), byday = p.studyDays.map(d => ['SU', 'MO', 'TU', 'WE', 'TH', 'FR', 'SA'][d]).join(',');
    let first = U.today(); for (let i = 0; i < 8 && !p.studyDays.includes(U.weekday(first)); i++) first = U.addDays(first, 1);
    const stamp = new Date().toISOString().replace(/[-:]/g, '').replace(/\.\d+/, '');
    // Floating local time (no TZID): the reminder stays at the same clock time wherever the Mac is.
    return ['BEGIN:VCALENDAR', 'VERSION:2.0', 'PRODID:-//LearnQuick//Spanish//EN', 'CALSCALE:GREGORIAN', 'BEGIN:VEVENT', `UID:learnquick-daily-${Date.now()}@local`, `DTSTAMP:${stamp}`,
      `DTSTART:${first.replace(/-/g, '')}T${hh}${mm}00`, 'DURATION:PT30M', `RRULE:FREQ=WEEKLY;BYDAY=${byday || 'MO,TU,WE,TH,FR'}`, 'SUMMARY:LearnQuick: práctica de español 🇪🇸',
      'DESCRIPTION:Abre LearnQuick y completa el plan de hoy. ¡Mantén tu racha!', 'BEGIN:VALARM', 'ACTION:DISPLAY', 'DESCRIPTION:LearnQuick', 'TRIGGER:PT0M', 'END:VALARM', 'END:VEVENT', 'END:VCALENDAR'].join('\r\n');
  }
  function restore(file) {
    // Checked before reading, so a huge file is never loaded into the page.
    if (file.size > LQ.store.MAX_BACKUP_BYTES) { LQ.ui.toast(L('That file is too large to be a LearnQuick backup.', 'Ese archivo es demasiado grande para ser una copia de LearnQuick.')); return; }
    const reader = new FileReader();
    reader.onload = () => {
      try {
        const S = LQ.store.parseBackup(reader.result);
        LQ.ui.modal(`<h2>${L('Restore this backup?', '¿Restaurar esta copia?')}</h2><p>${L(`Backup with ${S.done.length} plan days, ${Object.keys(S.units).length} lessons and ${Object.keys(S.cards).length} words. It will replace the progress currently on this Mac.`, `Copia con ${S.done.length} días del plan, ${Object.keys(S.units).length} lecciones y ${Object.keys(S.cards).length} palabras. Reemplazará el progreso actual de este Mac.`)}</p>
          <div class="bottom-buttons"><button type="button" class="primary-btn" data-action="restore-confirm">${L('Restore', 'Restaurar')}</button></div>`);
        LQ.pendingRestore = S;
      } catch (e) { LQ.ui.toast(L('That file could not be read as a LearnQuick backup.', 'Ese archivo no es una copia válida de LearnQuick.')); }
    };
    reader.readAsText(file);
  }
  LQ.restoreFile = restore;

  const set = (fn, rerender = true) => { fn(LQ.state); LQ.app.save(); if (rerender) LQ.app.render(); };
  Object.assign(LQ.actions, {
    'plan-days'(b) {
      const days = +b.dataset.v, S = LQ.state; if (days === S.plan.days) return;
      LQ.ui.modal(`<h2>${L(`Switch to the ${days}-day plan?`, `¿Cambiar al plan de ${days} días?`)}</h2><p>${L('Your lessons, words, streak and stats are kept. The day counter is recalculated from the lessons you have completed.', 'Se conservan tus lecciones, palabras, racha y estadísticas. El contador de días se recalcula a partir de las lecciones completadas.')}</p>
        <div class="bottom-buttons"><button type="button" class="primary-btn" data-action="plan-days-confirm" data-v="${days}">${L('Switch plan', 'Cambiar plan')}</button></div>`);
    },
    'plan-days-confirm'(b) { set(S => remapPlan(S, +b.dataset.v)); LQ.ui.closeModal(); LQ.ui.toast(L('Plan updated.', 'Plan actualizado.')); },
    'day-off-add'() { const v = document.getElementById('dayOffInput').value; if (v) set(S => { if (!S.plan.daysOff.includes(v)) S.plan.daysOff.push(v); }); },
    'day-off-remove'(b) { set(S => { S.plan.daysOff = S.plan.daysOff.filter(d => d !== b.dataset.v); }); },
    ics() { LQ.store.download('learnquick-reminder.ics', icsFile(LQ.state), 'text/calendar'); LQ.ui.toast(L('Open the downloaded file to add it to Calendar.', 'Abre el archivo descargado para añadirlo al Calendario.')); },
    backup() { LQ.store.exportBackup(LQ.state); LQ.ui.toast(L('Backup downloaded. Keep it in iCloud Drive or Documents.', 'Copia descargada. Guárdala en iCloud Drive o en Documentos.')); LQ.app.render(); },
    'restore-confirm'() {
      if (!LQ.pendingRestore) return;
      // replaceState() throws when the browser refuses the write; the progress on screen is then still the old one.
      try { LQ.app.replaceState(LQ.pendingRestore); }
      catch (e) { LQ.pendingRestore = null; LQ.ui.closeModal(); LQ.ui.toast(L('The backup could not be saved in this browser, so nothing was changed.', 'No se pudo guardar la copia en este navegador; no se ha cambiado nada.')); return; }
      LQ.pendingRestore = null; LQ.ui.closeModal(); LQ.ui.toast(L('Backup restored.', 'Copia restaurada.')); LQ.app.go('');
    },
    'reset-ask'() { LQ.ui.modal(`<h2>${L('Delete all progress?', '¿Borrar todo el progreso?')}</h2><p>${L('This cannot be undone. Download a backup first if you might want it back.', 'No se puede deshacer. Descarga antes una copia si quizá la quieras recuperar.')}</p><div class="bottom-buttons"><button type="button" class="secondary-btn" data-action="backup">${ico('download')}${L('Download backup', 'Descargar copia')}</button><button type="button" class="primary-btn danger" data-action="reset-confirm">${ico('trash', 'white')}${L('Delete everything', 'Borrar todo')}</button></div>`); },
    'reset-confirm'() {
      try { LQ.app.replaceState(P.defaultState(U.today())); }
      catch (e) { LQ.ui.closeModal(); LQ.ui.toast(L('Your progress could not be deleted: this browser refused the change.', 'No se pudo borrar el progreso: este navegador rechazó el cambio.')); return; }
      LQ.ui.closeModal(); LQ.app.go('onboarding');
    }
  });
  Object.assign(LQ.changes, {
    'plan-start'(el) { if (el.value) set(S => { S.plan.start = el.value; }); },
    'study-day'(el) { set(S => { const d = +el.value, list = new Set(S.plan.studyDays); el.checked ? list.add(d) : list.delete(d); if (!list.size) list.add(d); S.plan.studyDays = [...list].sort(); }); },
    'skip-holidays'(el) { set(S => { S.plan.skipHolidays = el.checked; }); },
    reminder(el) { set(S => { S.plan.reminder = el.value || '18:30'; }, false); },
    accent(el) { set(S => { S.profile.accent = el.value; S.profile.voiceURI = ''; }); },
    voice(el) { set(S => { S.profile.voiceURI = el.value; }, false); },
    rate(el) { set(S => { S.profile.rate = +el.value; }); },
    name(el) { set(S => { S.profile.name = el.value.trim().slice(0, 30); }, false); },
    'review-limit'(el) { set(S => { S.profile.reviewLimit = +el.value; }, false); },
    restore(el) { if (el.files && el.files[0]) restore(el.files[0]); el.value = ''; }
  });
})(typeof window !== 'undefined' ? window : globalThis);
