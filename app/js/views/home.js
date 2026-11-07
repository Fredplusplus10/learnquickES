/* Dashboard and Today's plan. */
(function (root) {
  'use strict';
  const LQ = root.LQ;
  const U = LQ.util, P = LQ.progress, C = LQ.content, CAL = LQ.calendar;
  const { ico, esc, wordRow, ring, meter, streakWeek } = LQ.ui;
  const L = (en, es) => LQ.L(en, es);
  const V = LQ.views = LQ.views || {};

  // Daily task catalogue: label, description, icon, colour tone and the route that runs it.
  LQ.tasks = {
    meta(task, st) {
      const S = LQ.state, due = P.totals(S, U.today()).words.due, u = st && st.unit, ph = st ? st.entry.phase : 1;
      const m = {
        review: [L('Review', 'Repaso'), due ? L(`${due} cards due · spaced repetition`, `${due} tarjetas pendientes · repaso espaciado`) : L('Nothing due today', 'Nada pendiente hoy'), 'refresh', 'blue', 'practice/review'],
        lesson: [u ? L(`Lesson ${u.n}: ${u.title}`, `Lección ${u.n}: ${u.es}`) : L('Lesson', 'Lección'), u ? u.canDo : '', 'play-square', '', u ? 'lesson/' + u.id : 'lessons'],
        verbs: [L('Verb gym · 80/20', 'Gimnasio de verbos · 80/20'), L('10 conjugations of the key verbs', '10 conjugaciones de los verbos clave'), 'bolt', '', 'practice/verbs'],
        mixed: [L('Mixed practice', 'Práctica mixta'), L('Exercises from the units you have done', 'Ejercicios de tus unidades'), 'target', 'purple', 'practice/mixed'],
        speak: [L('Speaking: shadowing', 'Hablar: shadowing'), L('Listen, repeat out loud, record yourself', 'Escucha, repite en voz alta, grábate'), 'mic', 'green', 'practice/speak'],
        cognates: [L('Cognates lab', 'Laboratorio de cognados'), L('-tion → -ción: words you already know', '-tion → -ción: palabras que ya sabes'), 'sparkles', '', 'practice/cognates'],
        falsefriends: [L('False friends', 'Falsos amigos'), L('Words that look English but are not', 'Palabras que parecen inglés y no lo son'), 'lightbulb', '', 'practice/falsefriends'],
        gender: [L('El / La trainer', 'Entrenador el / la'), L('Learn every noun with its article', 'Aprende cada sustantivo con su artículo'), 'cards', 'purple', 'practice/gender'],
        past: [L('Fui vs. Iba', 'Fui vs. iba'), L('The real B1 filter: narrating the past', 'El filtro real del B1: narrar el pasado'), 'clock', 'blue', 'practice/past'],
        describe: [L('Describe it in Spanish', 'Descríbelo en español'), L('Language-island skill: no English allowed', 'Habilidad de la isla: prohibido el inglés'), 'palm', 'green', 'practice/describe'],
        checkpoint: [L(`Checkpoint · Phase ${ph}`, `Control · Fase ${ph}`), L('15 questions from this phase', '15 preguntas de esta fase'), 'award', '', 'checkpoint/' + ph],
        final: [L('B1 mock exam', 'Simulacro B1'), L('Reading, listening, grammar, writing and speaking', 'Lectura, audio, gramática, escritura y expresión oral'), 'award', '', 'checkpoint/5']
      }[task];
      return { name: m[0], desc: m[1], icon: m[2], tone: m[3], route: m[4], min: CAL.MINUTES[task] || 5 };
    },
    row(task, st, done, big) {
      const m = LQ.tasks.meta(task, st);
      return `<button type="button" class="task-row ${big ? 'big' : ''} ${done ? 'is-done' : ''}" data-go="${m.route}">
        <div class="task-ico ${m.tone}">${ico(m.icon)}</div>
        <div class="task-text"><b>${esc(m.name)}</b><small>${esc(m.desc)}</small></div>
        <div class="right">${done ? `<span class="done-check" title="${L('Done', 'Hecho')}">${ico('check', 'white')}</span>` : `~${m.min} min ${ico('chevron-right')}`}</div>
      </button>`;
    }
  };

  const doneToday = S => S.done.some(d => d.date === U.today());
  function paceText(p) {
    if (p.diff > 0) return L(`${p.diff} day${p.diff > 1 ? 's' : ''} ahead`, `${p.diff} día${p.diff > 1 ? 's' : ''} adelantado`);
    if (p.diff === 0) return L('on track', 'al día');
    return L(`${-p.diff} behind — just continue, no stress`, `${-p.diff} de retraso: sigue, sin estrés`);
  }
  function banners(S, today) {
    const out = [];
    if (LQ.store.demo) out.push(`<div class="banner info">${ico('eye')}<span>${L('Demo data: this is a sample learner. Your real progress is stored separately — open the app without "?demo".', 'Datos de demostración: es un alumno de ejemplo. Tu progreso real se guarda aparte; abre la app sin «?demo».')}</span><a class="secondary-btn" href="index.html">${L('Exit demo', 'Salir de la demo')}</a></div>`);
    if (!LQ.store.available()) out.push(`<div class="banner warn">${ico('bell')}<span>${L('This browser is not saving data (private window or blocked storage). Use a normal Safari or Chrome window.', 'Este navegador no está guardando datos (ventana privada o almacenamiento bloqueado). Usa una ventana normal de Safari o Chrome.')}</span></div>`);
    else if (!LQ.store.demo && S.done.length >= 3 && (!S.lastBackup || U.diffDays(S.lastBackup, today) >= 7)) out.push(`<div class="banner">${ico('download')}<span>${L('Keep your progress safe: download a backup file. It takes one click.', 'Protege tu progreso: descarga una copia de seguridad. Es un clic.')}</span><button type="button" class="secondary-btn" data-action="backup">${L('Download backup', 'Descargar copia')}</button></div>`);
    return out.join('');
  }

  V.dashboard = () => {
    const S = LQ.state, today = U.today(), st = P.planStatus(S), pace = P.pace(S, today), si = P.streakInfo(S, today), T = P.totals(S, today);
    const phase = C.phases[st.entry.phase - 1], pct = st.doneCount / st.total * 100, name = S.profile.name ? `, ${esc(S.profile.name)}` : '';
    const finishedToday = doneToday(S);
    const focus = st.finished ? L('You completed the whole plan. Keep your streak alive with reviews and immersion.', 'Completaste todo el plan. Mantén la racha con repasos e inmersión.')
      : st.entry.type === 'lesson' ? L(`Today: Lesson ${st.unit.n} — ${st.unit.title}.`, `Hoy: Lección ${st.unit.n}, ${st.unit.es}.`)
        : st.entry.type === 'checkpoint' ? L(`Today: Phase ${st.entry.phase} checkpoint.`, `Hoy: control de la fase ${st.entry.phase}.`)
          : L('Today: practice day — review, verbs and a focused trainer.', 'Hoy: día de práctica, con repaso, verbos y un entrenador.');
    const tasks = st.finished ? [] : st.tasks;
    const nextTask = tasks.find(t => !P.taskDone(S, st.n, t));
    const words = st.unit ? st.unit.vocab.slice(0, 5) : LQ.srs.queue(S.cards, today, 5).map(id => C.vocabIndex[id]).filter(Boolean);
    const wordsFallback = words.length ? words : Object.keys(S.cards).slice(-5).map(id => C.vocabIndex[id]).filter(Boolean);
    const trick = C.tricks[Math.floor(U.seeded(today)() * C.tricks.length)];
    const acc = T.accuracy('verbs');
    return `${banners(S, today)}
    <section class="hero"><div class="hero-copy"><h1><span class="flag" aria-hidden="true"><img src="assets/icons/spanish-flag.svg" alt=""></span> ¡Hola${name}!</h1>
      <p class="lead">${st.finished ? L('Plan complete!', '¡Plan completado!') : L(`Day <b>${st.n} of ${st.total}</b> · ${esc(phase.en)}`, `Día <b>${st.n} de ${st.total}</b> · ${esc(phase.es)}`)}</p>
      <p class="sub">${esc(focus)} ${st.finished ? '' : L(`About ${st.minutes} minutes in the app, plus ${CAL.INPUT_TARGET[S.plan.days]} minutes of Spanish listening.`, `Unos ${st.minutes} minutos en la app y ${CAL.INPUT_TARGET[S.plan.days]} minutos de escucha en español.`)}</p></div>
      <img class="hero-art" src="assets/illustrations/spanish-city.svg" alt="" aria-hidden="true"></section>
    <section class="grid top-grid" aria-label="${L('Progress overview', 'Resumen del progreso')}">
      <div class="card soft"><div class="progress-content"><div class="progress-main"><p class="eyebrow">${L('Your progress', 'Tu progreso')}</p>
        <div class="metric-big">${st.finished ? L('All days done', 'Todos los días hechos') : L(`Day ${st.n} of ${st.total}`, `Día ${st.n} de ${st.total}`)}</div>
        <p class="metric-small">${L(`${pace.remaining} study days to go`, `Faltan ${pace.remaining} días de estudio`)} · ${paceText(pace)}</p>
        ${meter(pct, L('Plan completion', 'Plan completado'))}
        <div class="goal">${ico('target')}<span><b>${L('Goal:', 'Meta:')}</b> ${L('functional B1 Spanish', 'español B1 funcional')}${pace.target ? ` · ${L('target', 'fecha objetivo')} ${U.fmtDate(pace.target, LQ.lang())}` : ''}</span></div></div>
        ${ring(pct, L('Complete', 'Completado'))}</div></div>
      <div class="card streak-card"><div class="streak-head">${ico('flame')}<div><h2>${L(`${si.count}-day streak`, `Racha de ${si.count} días`)}</h2><p>${si.todayDone ? L('Today is done. ¡Hasta mañana!', 'Hoy ya está. ¡Hasta mañana!') : L('Earn 10 XP today to keep it going.', 'Gana 10 XP hoy para mantenerla.')}</p></div></div>
        ${streakWeek(si.week)}
        <p class="freeze-line">${ico('shield')} ${L(`${si.freezes}/2 streak freezes`, `${si.freezes}/2 protectores`)} <small>· ${L('weekends never break it', 'los fines de semana no la rompen')}</small></p></div>
    </section>
    <section class="grid mid-grid" aria-label="${L('Daily learning', 'Aprendizaje diario')}">
      <div class="card" id="daily-plan"><div class="card-head"><h2 class="card-title">${ico('calendar')}${L("Today's plan", 'Plan de hoy')}</h2><span class="tag">~${st.minutes} min</span></div>
        <p class="card-sub">${finishedToday ? L('Done for today ✓ — the next day is ready if you want to get ahead.', 'Listo por hoy ✓. El siguiente día está disponible si quieres adelantar.') : L('Complete these to finish the day and move forward.', 'Complétalas para terminar el día y avanzar.')}</p>
        ${tasks.map(t => LQ.tasks.row(t, st, P.taskDone(S, st.n, t))).join('') || `<p class="card-sub">${L('No tasks left in the plan.', 'No quedan tareas en el plan.')}</p>`}</div>
      <div class="card" id="vocab-preview"><div class="card-head"><h2 class="card-title">${ico('book')}${st.unit ? L("Today's words", 'Palabras de hoy') : L('Words to review', 'Palabras para repasar')}</h2><button type="button" class="text-link" data-go="vocab">${L('See all ›', 'Ver todas ›')}</button></div>
        <p class="card-sub">${st.unit ? L(`From Lesson ${st.unit.n}. Tap the speaker to listen.`, `De la lección ${st.unit.n}. Toca el altavoz para escuchar.`) : L('Due in your spaced-repetition deck.', 'Pendientes en tu mazo de repaso.')}</p>
        ${wordsFallback.length ? wordsFallback.map(wordRow).join('') : `<p class="card-sub">${L('Your words will appear here after your first lesson.', 'Tus palabras aparecerán aquí después de tu primera lección.')}</p>`}</div>
      <div class="card"><h2 class="card-title">${ico('bolt')}${L('Quick actions', 'Acciones rápidas')}</h2><div class="actions">
        ${nextTask ? `<button type="button" class="primary-btn" data-go="${LQ.tasks.meta(nextTask, st).route}">${ico('play-square', 'white')}${finishedToday ? L('Get ahead', 'Adelantar') : L('Continue today', 'Seguir con hoy')}</button>` : `<button type="button" class="primary-btn" data-go="practice/review">${ico('refresh', 'white')}${L('Review cards', 'Repasar tarjetas')}</button>`}
        <button type="button" class="secondary-btn" data-go="practice/review">${ico('cards')}${L(`Flashcards · ${T.words.due} due`, `Tarjetas · ${T.words.due} pendientes`)}</button>
        <button type="button" class="secondary-btn" data-go="practice/island">${ico('palm')}${L('Spanish island', 'Isla lingüística')}</button>
        <button type="button" class="secondary-btn" data-go="immersion">${ico('headphones')}${L('Log listening', 'Registrar escucha')}</button></div></div>
    </section>
    <section class="card stats" aria-label="${L('Your statistics', 'Tus estadísticas')}"><div class="stat-heading">${ico('chart')}<div><b>${L('Your stats', 'Tus estadísticas')}</b><small>${L('Measured from your sessions', 'Medido en tus sesiones')}</small></div></div>
      <div class="stat blue">${ico('clock')}<div><b>${(T.spanishMin / 60).toFixed(1)} h</b><small>${L('in Spanish (app + listening)', 'en español (app + escucha)')}</small></div></div>
      <div class="stat green">${ico('cards')}<div><b>${T.words.remembered}</b><small>${L(`words remembered of ${T.words.seen}`, `palabras recordadas de ${T.words.seen}`)}</small></div></div>
      <div class="stat purple">${ico('bolt')}<div><b>${acc === null ? '—' : Math.round(acc * 100) + '%'}</b><small>${L('verb accuracy', 'acierto en verbos')}</small></div></div></section>
    <section class="card trick-card"><div class="trick-num">${L('Trick', 'Truco')} ${trick.n}</div><div><b>${esc(trick.title)}</b><p>${esc(trick.text)}</p></div><button type="button" class="secondary-btn" data-go="${trick.route}">${L('Try it', 'Probar')} ${ico('chevron-right')}</button></section>`;
  };

  V.today = () => {
    const S = LQ.state, today = U.today(), st = P.planStatus(S), T = P.totals(S, today), a = S.activity[today] || { input: 0, island: 0 };
    const typeLabel = { lesson: L('Lesson day', 'Día de lección'), practice: L('Practice day', 'Día de práctica'), checkpoint: L('Checkpoint day', 'Día de control') }[st.entry.type];
    const target = CAL.INPUT_TARGET[S.plan.days], inputToday = a.input + a.island + (a.talk || 0), finishedToday = doneToday(S);
    const offDay = !CAL.isStudyDay(today, S.plan);
    const date = U.fmtDate(today, LQ.lang(), { weekday: 'long', month: 'long', day: 'numeric' });
    return `<section class="page-head"><p class="eyebrow">${esc(date)}</p><h1>${st.finished ? L('Plan complete', 'Plan completado') : L(`Day ${st.n} of ${st.total} · ${typeLabel}`, `Día ${st.n} de ${st.total} · ${typeLabel}`)}</h1>
      <p class="sub">${L(`Phase ${st.entry.phase}: ${C.phases[st.entry.phase - 1].en}. About ${st.minutes} minutes in the app.`, `Fase ${st.entry.phase}: ${C.phases[st.entry.phase - 1].es}. Unos ${st.minutes} minutos en la app.`)}</p></section>
    ${offDay ? `<div class="banner info">${ico('calendar')}<span>${L('Today is a day off in your plan. Studying is optional — and it earns you a streak freeze.', 'Hoy es día libre en tu plan. Estudiar es opcional, y te da un protector de racha.')}</span></div>` : ''}
    ${finishedToday && !st.finished ? `<div class="banner success">${ico('check')}<span>${L(`Today's plan is complete. Want to get ahead? Day ${st.n} is ready below.`, `El plan de hoy está completo. ¿Quieres adelantar? El día ${st.n} está abajo.`)}</span></div>` : ''}
    <section class="card"><div class="card-head"><h2 class="card-title">${ico('calendar')}${L('Tasks', 'Tareas')}</h2><span class="tag">~${st.minutes} min</span></div>
      <p class="card-sub">${L('Finish every task to complete the day. Each one opens a short session.', 'Termina cada tarea para completar el día. Cada una abre una sesión corta.')}</p>
      ${st.finished ? LQ.ui.emptyState('award', L('You finished all the days!', '¡Terminaste todos los días!'), L('Keep reviewing and listening every day to lock in your B1.', 'Sigue repasando y escuchando cada día para consolidar tu B1.'), '', 'empty-plan-done') : st.tasks.map(t => LQ.tasks.row(t, st, P.taskDone(S, st.n, t), true)).join('')}</section>
    <section class="grid two-grid">
      <div class="card"><h2 class="card-title">${ico('headphones')}${L('Immersion goal', 'Meta de inmersión')}</h2>
        <p class="card-sub">${L(`The app is the core; real progress to B1 needs volume. Target: ${target} min of Spanish input a day (podcasts, series, island, exchanges).`, `La app es el núcleo; llegar a B1 necesita volumen. Meta: ${target} min de español al día (pódcasts, series, isla, intercambios).`)}</p>
        <div class="metric-row"><b>${Math.round(inputToday)} / ${target} min</b>${meter(inputToday / target * 100, L('Immersion today', 'Inmersión hoy'))}</div>
        <div class="quick-log">${[15, 30, 60].map(m => `<button type="button" class="pill" data-action="log-input" data-min="${m}">+${m} min</button>`).join('')}<button type="button" class="text-link" data-go="immersion">${L('Details ›', 'Detalles ›')}</button></div></div>
      <div class="card"><h2 class="card-title">${ico('palm')}${L('Spanish island', 'Isla lingüística')}</h2>
        <p class="card-sub">${L('Trick 2: one hour a day with zero English. The app switches to Spanish and times it for you.', 'Truco 2: una hora al día sin inglés. La app cambia a español y te lo cronometra.')}</p>
        <p class="metric-row"><b>${Math.round(T.islandMin / 60 * 10) / 10} h</b> <small>${L('total on the island', 'en total en la isla')}</small></p>
        <button type="button" class="secondary-btn" data-go="practice/island">${ico('palm')}${L('Start an island session', 'Empezar una sesión')}</button></div>
    </section>`;
  };
})(typeof window !== 'undefined' ? window : globalThis);
