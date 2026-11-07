# Implementation status

What the **web application in `app/`** includes today, and what it leaves out. The native desktop app is described in [desktop/README.md](../desktop/README.md).

## Included (v1.0, local web app)

- Onboarding with name, interface language, 15-question placement test, plan recommendation, voice setup and a first mission.
- Study plans of 30, 60 or 90 study days. Study weekdays are configurable, and US federal holidays (observed-date rule) and custom days off are excluded. Pace, days behind and target date are calculated from completed days.
- 25 lessons in 5 phases. Each has 12 vocabulary items with articles, a grammar hack with tables and examples, a dialogue with audio, exercises (multiple choice, fill-in, word order, listening, matching) and speaking prompts.
- Trainers: verb conjugation (25 verbs, 8 tenses unlocked by lesson progress), preterite vs. imperfect, cognates, false friends, noun gender, describe-it, mixed practice and spaced-repetition review.
- A streak computed from recorded activity: 10 XP per day, up to 2 freezes, and non-study days never break it.
- Immersion log with daily targets, hour milestones, resources, device-language missions, language exchanges and a timed language island that switches the UI to Spanish.
- Phase checkpoints and a B1 mock exam modelled on the DELE B1 structure.
- Progress metrics derived from recorded events: minutes per day, accuracy by skill, recent streak calendar, B1 readiness signals and 16 achievements.
- Settings: plan length, start date, study days, holidays, days off, `.ics` reminder, accent, voice and speed, UI language, vosotros, review limit, backup, restore and reset.
- Versioned local persistence with migration of older states, weekly backup reminder and JSON export/import.
- 20 automated tests (Node `node:test`) and a static repository validation script, both run in CI.

## Not included

- A finished native macOS `.app`, code signing or notarization. The C++20 + Qt 6 app in `desktop/` is at phase 3: the daily plan runs end to end (lessons, review, trainers, checkpoints, B1 mock); the browsing screens and backup import come in phase 4; CI builds it unsigned.
- SQLite persistence (planned for the native app), user accounts or cloud sync (by design: single learner, local only).
- Automatic pronunciation assessment. Recording and browser speech recognition appear only where the browser supports them, and no score is invented.
- Adaptive sequencing. The plan is fixed; the only adaptation is that wrong answers come back later in the session and checkpoints point to weak units.
- Independent validation of learning outcomes, or a formal accessibility audit.
- Testing on a real Mac. It was developed and tested on Windows with headless Chrome. Safari storage on `file://`, installed voices and microphone access still have to be checked on the learner's Mac.

## Demo-only content

`app/index.html?demo` loads a fictional learner ("Cory", day 16 of 60) stored under a separate key. Every number shown in demo mode and in the README screenshots is sample data, not a measured result.

A 30–90-study-day plan defines duration. It is not a clinically or educationally validated proficiency outcome.
