# Content and persistence model

## Current persistence (v1.0)

The whole learner state is one JSON object saved in `localStorage` under the key `learnquick.v1` (the demo uses `learnquick.demo.v1`). The same object is what *Settings → Download backup* exports. `progress.migrate()` fills in fields added later, so older backups still load.

| Field | Content |
| --- | --- |
| `version`, `createdAt`, `onboarded` | Schema version and lifecycle |
| `profile` | Name, level, placement score, accent, voice, speech rate, UI language, vosotros, review limit |
| `plan` | `days` (30 / 60 / 90), `start`, `studyDays` (weekday numbers), `skipHolidays`, `daysOff`, `reminder` |
| `done` | Completed plan days (dates) |
| `tasks` | Per plan day, which tasks were completed and when |
| `units` | Per lesson: first completion date, last score, best score, attempts |
| `checkpoints` | Per phase (5 = B1 mock): first date, last score, best score |
| `cards` | Spaced-repetition state per vocabulary id: `box`, `due`, `reps`, `lapses`, `added`, `last`, `u` (unit) |
| `activity` | Per calendar day: `xp`, `sec` (active app time), `input`, `island`, `talk` (minutes), `speak`, and `ex` (answers per skill: attempts / correct) |
| `streak` | `freezes` held, `frozen` (days covered by a freeze), `longest` |
| `immersion`, `exchanges` | Listening log entries and language-exchange sessions |
| `missions`, `achievements`, `writing` | Date each device mission or achievement was done; mock-exam writing (date, word count, text) |
| `lastBackup` | Date of the last exported backup |

Dates are local calendar dates (`YYYY-MM-DD`). This is deliberate: a study day is a day in the learner's own calendar, not a UTC interval. Streak, accuracy, time in Spanish and readiness signals are derived from `activity` and `done` when needed.

Lesson content is plain JavaScript data in `app/js/content/` (not JSON) because browsers block `fetch` on `file://`. The sample JSON below shows the format a native build could import.

## Core entities (proposed for a native build)

| Entity | Representative fields | Purpose |
| --- | --- | --- |
| `LearningPlan` | id, duration_days, start_date, timezone, holiday_policy | Study commitment and scheduling |
| `Lesson` | id, version, unit_id, title, locale | Immutable learning content metadata |
| `Exercise` | id, lesson_id, type, prompt, answer_spec | Explicit assessment input |
| `StudySession` | id, started_at, ended_at, lesson_id | Bounded user activity |
| `AnswerEvent` | id, session_id, exercise_id, value, is_correct, timestamp | Recomputable assessment history |
| `VocabularyItem` | id, lemma, translation, context | Flashcard and lexical data |
| `ReviewState` | vocabulary_id, due_at, interval, ease | Spaced repetition |
| `Settings` | language, speech_rate, accessibility_prefs | Local preferences |

## Business-day semantics

The original prototype counted Monday through Friday only. The v1.0 app keeps three concepts separate, as recommended here: an **eligible study day** (calendar policy: chosen weekdays, minus US federal holidays and days off), a **completed plan day** (every task of that plan day done, stored in `done`), and an **active streak day** (at least 10 XP in `activity`). Pace and the target date use eligible days remaining, and freezes cover missed eligible days only.

- Store study timestamps in UTC and display them in the configured local timezone.
- Persist locale/timezone and a versioned holiday calendar when excluding holidays.
- Handle study breaks as policy decisions, not arithmetic errors.
- Recalculate an ETA from remaining eligible days, not from UI animation frames.

## Data integrity

Use foreign keys, unique identifiers, database transactions, schema migrations and validated input. Any cached analytics must be derivable from primary events. Do not store raw microphone audio by default without an explicit product requirement and retention policy.

## Example content

The sample café lesson is included at `content/lessons/lesson-12-cafe.sample.json`. It is an illustrative input file; no production schema validation or import pipeline has been implemented yet.
