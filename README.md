<p align="center">
  <img src="assets/preview/readme-header.png" alt="LearnQuick: Spanish to functional B1 in 30–90 days, for Windows, macOS and the web" width="100%">
</p>

# LearnQuick

**An offline-first Spanish learning app for macOS and Windows: a personal 30-, 60- or 90-study-day plan towards functional B1 Spanish, with a Duolingo-style streak.**

LearnQuick is for any English speaker who wants functional Spanish: each person installs it, answers a short welcome and gets their own plan. Its first learner is an English speaker on a MacBook. Each day combines a short session in the app (review, a lesson or a trainer, speaking out loud) with Spanish input outside it (podcasts, series, conversation). The app tracks both.

> **Current status: working local web app (v1.0); native app in progress.** The web app is a dependency-free HTML/CSS/JavaScript application that runs from a double-click, with no server, account or network connection. The native desktop app is being built in C++20 + Qt 6 in [`desktop/`](desktop/README.md). It is at phase 4 of 5: the whole daily plan runs natively (lessons, spaced-repetition review, the eight trainers, phase checkpoints and the B1 mock), with progress in SQLite, a built-in neural Spanish voice, a guiding coach that asks every few sessions what the learner enjoyed, and screens for words, immersion and progress with achievements, built for Windows and macOS in CI (see [roadmap](docs/ROADMAP.md)).

## Interface preview

### Desktop app on Windows

| Today | Lesson |
| :---: | :---: |
| ![Windows: today's plan, streak, coach and progress](assets/screenshots/windows-today.png) | ![Windows: lesson 8, the dialogue step](assets/screenshots/windows-lesson-7-dialogue.png) |
| **Practice** | **Progress** |
| ![Windows: the trainers, with the review pile first](assets/screenshots/windows-practice.png) | ![Windows: goal, target date, pace and roadmap](assets/screenshots/windows-progress.png) |

*Windows 11, LearnQuick 0.4.5, light theme. Real screenshots of the desktop app with its built-in sample learner (`--demo --today 2026-11-02`); the values are demo data, not measured learning results.*

### Desktop app on macOS

| Today | Lesson |
| :---: | :---: |
| ![macOS: today's plan, streak, coach and progress](assets/screenshots/macos-today.png) | ![macOS: lesson 8, the dialogue step](assets/screenshots/macos-lesson-7-dialogue.png) |
| **Practice** | **Progress** |
| ![macOS: the trainers, with the review pile first](assets/screenshots/macos-practice.png) | ![macOS: goal, target date, pace and roadmap](assets/screenshots/macos-progress.png) |

*macOS 26 Tahoe, LearnQuick 0.4.5, light theme, taken by the build workflow with the same sample learner and the same demo data.*

<details>
<summary>Web app</summary>

| Dashboard | Lesson |
| :---: | :---: |
| ![Dashboard with plan, streak and daily tasks](assets/screenshots/app-dashboard.webp) | ![Lesson 14, fui vs. iba grammar hack](assets/screenshots/app-lesson.webp) |
| **Practice** | **Progress** |
| ![Practice hub with the trainers](assets/screenshots/app-practice.webp) | ![Progress: roadmap, minutes per day and accuracy](assets/screenshots/app-progress.webp) |

*Real screenshots of the web app with its sample learner (`?demo`), captured in headless Chrome.*

</details>

## Project scope

| Capability | Status | Notes |
| --- | --- | --- |
| Study plan of 30 / 60 / 90 study days | Implemented | Mon–Fri by default; weekends, US federal holidays and custom days off are excluded. Pace and target date are recalculated from completed days |
| Placement test and plan recommendation | Implemented | 15 questions → A1 / A2 / A2+ and a suggested plan length |
| Curriculum | Implemented | 25 lessons in 5 phases; 300 vocabulary items with articles; 230 exercises; dialogues; grammar “hacks” |
| Streak | Implemented | 10 XP keeps the day; up to 2 freezes; non-study days never break it; the daily goal is separate from the streak |
| Trainers | Implemented | Verb conjugation (25 verbs, tenses unlock with lessons), preterite vs. imperfect, cognates, false friends, noun gender, describe-it, mixed practice |
| Spaced repetition | Implemented | Leitner boxes (0, 1, 2, 4, 8, 16, 32, 60 days); accent-only mistakes count as “almost” |
| Immersion tracking | Implemented | Listening log with daily target, hour milestones, curated A2–B1 resources, device-language missions, language exchanges, timed “language island” that switches the UI to Spanish |
| Checkpoints and B1 mock exam | Implemented | 15-question checkpoint per phase; final mock modelled on DELE B1 (writing and speaking partly self-assessed) |
| Spanish text-to-speech | Browser-dependent | `SpeechSynthesis` with installed macOS voices; Mexican Spanish by default, configurable |
| Built-in neural voice | Implemented (native app) | A natural Spanish voice (Supertonic 2) that ships with the app and works offline, the same on Windows and Mac, at a pace for learners, with a voice for each dialogue character; the learner picks their own voice (six) and its normal and slow speed in Settings; "a / b" is read "a o b" and "305" "trescientos cinco" |
| Recording and speech recognition | Browser-dependent | Shown only when the browser allows it; no pronunciation score is ever invented. “Listen & repeat” works everywhere |
| Persistence | Implemented (browser) | `localStorage` with versioned schema, weekly backup reminder, JSON export/import |
| Calendar reminder | Implemented | Downloads a repeating `.ics` event for the study days |
| Native app for macOS and Windows | In progress | C++20 + Qt 6 in [`desktop/`](desktop/README.md). CI builds an unsigned universal `.dmg` (not notarized) and a Windows folder with the `.exe` |
| SQLite persistence | Implemented (native app) | Profile, full progress and settings in a local SQLite file with versioned migrations; backups in the web app's format, so either app restores the other's |
| Day and night mode | Implemented (native app) | Follows the computer by default; Settings or the sun / moon button switches it |

The 30–90-day range is a **study-plan duration, not a guarantee of fluency**. From a solid A2 base, the plan targets functional conversational B1; from zero, a solid A2 is the realistic outcome in 90 days. The reasoning and sources are in [docs/INVESTIGACION-Y-METODO.md](docs/INVESTIGACION-Y-METODO.md) (Spanish).

## Run it

No frameworks, package manager, build step, external APIs or network services are required.

**On the learner's Mac:** double-click [`LearnQuick.html`](LearnQuick.html). It opens `app/index.html` in the default browser (Safari recommended). The student guide is [docs/STUDENT-GUIDE.md](docs/STUDENT-GUIDE.md).

**Demo data:** open `app/index.html?demo` (or click *Explore a demo* on the welcome screen). The demo uses separate storage and never touches real progress.

**Optional local server** (lets Chrome use the microphone, which it blocks on `file://`):

```bash
python3 -m http.server 8765
```

Then open `http://localhost:8765/app/`. Browser storage is per origin, so progress saved under `file://` and under `localhost` is separate; move it with *Settings → Download backup / Restore*.

## Installing a build that isn't signed yet

LearnQuick isn't signed with a paid developer certificate yet, so your system warns you the first time you open it. The app works offline, has no account and never connects to the internet.

**Windows.** Extract the whole zip first, then open `LearnQuick.exe`. If a blue *Windows protected your PC* window appears, click **More info → Run anyway**. Each new version asks once.

**macOS.** Drag LearnQuick to Applications and open it. If macOS says *Apple could not verify "LearnQuick"*:
- **macOS 15 or later:** click **Done**, open **System Settings → Privacy & Security**, scroll down, click **Open Anyway** and confirm with your password.
- **macOS 14:** right-click LearnQuick → **Open** → **Open**.

If it says the app *is damaged*, run `xattr -dr com.apple.quarantine /Applications/LearnQuick.app` in Terminal and open it again.

Step by step, with updating, uninstalling and where your progress is kept: [Installing LearnQuick](docs/INSTALL.md).

## Learning method

The app implements five practical “tricks”, each tied to concrete features:

| Trick | Where it lives in the app |
| --- | --- |
| 1. 80/20 — the most frequent verbs (ser, estar, tener, hacer, poder, querer, ir, decir) and cognates | Phase 1, verb gym, cognates lab, false friends |
| 2. Goodbye English — device language and a daily “language island” | Device missions, one-hour island timer (whole UI in Spanish), describe-it trainer |
| 3. Comprehensible input at A2–B1 | Listening log and targets, hour milestones, curated resources, Spanish-subtitle guidance |
| 4. Speak from day one | Speaking step in every lesson, shadowing and pronunciation gym, language exchanges |
| 5. Hack the grammar — fui vs. iba, nouns with their article, subjunctive for recognition only | Preterite/imperfect trainer, gender trainer, a “Hack” box in every lesson |

## Architecture

### Current implementation

```text
LearnQuick.html  ──►  app/index.html  (design-kit shell, classic <script> tags)
                          │
                          ├── js/core/      pure logic: calendar, streak, SRS, verbs, progress, store
                          ├── js/content/   lessons, trainers, resources, mock exam (plain JS data)
                          ├── js/ui/        components, speech, exercise session engine
                          ├── js/views/     one module per screen (hash routing)
                          └── js/app.js     router, delegated events, top bar, ⌘K search, activity timer
                          │
                          ▼
                    localStorage (versioned state) + JSON backups
```

Classic scripts are used instead of ES modules and `fetch` because browsers block both on `file://`. The same files load in Node for the tests. Details: [architecture](docs/ARCHITECTURE.md) and [data model](docs/DATA_MODEL.md).

### Native app (in progress)

The native app is **C++20 + Qt 6**: QML describes the screens and C++ holds all the logic. It adds SQLite, and later on-device speech recognition (whisper.cpp) and conversation practice (llama.cpp). It is developed on Windows, and CI builds a universal macOS binary that runs on Apple Silicon and Intel. The web app's content and tests are its specification. Build instructions and phases: [`desktop/README.md`](desktop/README.md).

## Repository structure

```text
.
├── README.md
├── LearnQuick.html        # Launcher: double-click to open the app
├── app/                   # The application (HTML/CSS/JS, no dependencies)
│   ├── index.html
│   ├── css/               # kit.css (design system) + app.css (application layer)
│   ├── js/                # core / content / ui / views / app.js
│   └── assets/            # Icons, illustrations, logos, app icons
├── tests/                 # node:test suites for the learning engine and content
├── assets/
│   ├── brand/             # Scalable visual identity and application icon
│   ├── preview/           # GitHub social preview and README header
│   └── screenshots/       # Real screenshots of the web and desktop apps
├── content/lessons/       # Example structured lesson content (future JSON format)
├── desktop/               # Native app: C++20 + Qt 6 (CMake, QML, platform icons)
├── docs/                  # Architecture, method, guides, roadmap and policies
├── scripts/               # Static repository validation
└── .github/               # CI, issue and pull request templates
```

## Documentation

- [Student guide](docs/STUDENT-GUIDE.md) (English)
- [Count the Spanish you watch on Netflix](docs/NETFLIX-GUIDE.md) (English, desktop app, Windows and Mac)
- [Research, conclusions and method](docs/INVESTIGACION-Y-METODO.md) (Spanish)
- [Developer guide](docs/DESARROLLO.md) (Spanish)
- [Architecture and component boundaries](docs/ARCHITECTURE.md)
- [Content and persistence model](docs/DATA_MODEL.md)
- [UI design system and brand rules](docs/DESIGN_SYSTEM.md)
- [Implementation status and limitations](docs/STATUS.md)
- [Development roadmap](docs/ROADMAP.md)
- [Native desktop app: build and phases](desktop/README.md) (Spanish)
- [Brand asset inventory](docs/ASSET_GUIDELINES.md)

## Quality checks

```bash
python3 scripts/validate_repository.py
node --test tests/core.test.js tests/content.test.js
```

The validation script checks SVG files, required repository artifacts, local HTML/CSS asset references and Markdown relative links. The test suites cover calendar and holiday rules, streak and freeze logic, spaced repetition, answer checking, every verb × tense form, a full simulated 30-day plan and the integrity of all 25 lessons. GitHub Actions runs both on every push, plus a JavaScript syntax check.

For the native app, a second workflow runs whenever `desktop/` changes:
- it builds on Windows and macOS;
- it runs a headless smoke test, which fails on any QML warning, on both the build and the deployed app;
- it publishes the Windows build and a universal `.dmg` as downloadable artifacts.

## Privacy and distribution

The app includes no external fonts, SDKs, analytics or telemetry, and makes no network requests. Progress stays in the browser's local storage on the learner's Mac and can be exported as a JSON file. Links to external resources (podcasts, Tandem, HelloTalk) open only when the learner clicks them. If the browser offers speech recognition, it may rely on the vendor's online service; the feature is optional.

## License

- **Code:** the MIT License ([LICENSE](LICENSE)).
- **Graphic assets: all rights reserved.** The MIT License does not cover the illustrations, logos, app icons, interface icons and certificate designs. You may not reuse them outside LearnQuick without permission. They are:
  - `assets/brand/` and `assets/preview/`;
  - `app/assets/illustrations/`, `app/assets/logos/`, `app/assets/app-icons/` and `app/assets/icons/`;
  - `desktop/images/`, `desktop/assets/illustrations/` and the app icons in `desktop/platform/`;
  - the certificate designs (`desktop/assets/certificates/`: `certificate_*.svg` and their level badges).
- **Third-party software** in the desktop app (Qt, FFmpeg, ONNX Runtime, the Supertonic 2 voice and others) keeps its own licenses: see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). The built-in voice is under the BigScience Open RAIL-M License, whose use restrictions apply to anyone using it through LearnQuick.

More detail: [licensing notes](docs/LICENSING.md).

## Contributing

Review [CONTRIBUTING.md](CONTRIBUTING.md) before proposing changes.
