# Engineering roadmap

## Milestone 0 — Design specification

- [x] Visual identity and application icon
- [x] Three conceptual macOS application views
- [x] Navigation and interaction prototype
- [x] Repository documentation and GitHub-ready assets
- [ ] User research / formal curriculum review

## Milestone 1 — Local learning application (current, v1.0)

- [x] Dependency-free app that opens from a double-click (`LearnQuick.html`)
- [x] Plan scheduling with an explicit eligible-day policy (study weekdays, US federal holidays, days off)
- [x] 25-lesson curriculum, trainers and checkpoint / B1 mock exam
- [x] Answer-event recording and progress metrics derived from activity
- [x] Streak with freezes, separated from the daily goal
- [x] Text-to-speech with installed macOS voices
- [x] Vocabulary recall with spaced repetition (Leitner)
- [x] Versioned local state, migration, backup and restore
- [x] Unit tests and CI
- [ ] Verification on the learner's Mac (Safari storage on `file://`, voices, microphone)

## Milestone 2 — Native application (C++20 + Qt 6, in progress)

- [x] Choose the native path: C++20 + Qt 6 with QML (see [architecture](ARCHITECTURE.md))
- [x] Phase 1: CMake project, Qt Quick window, `.icns` / `.ico` icons, Windows build and universal macOS `.dmg` in CI, with a headless smoke test
- [x] Phase 2: C++ learning engine (dates, US holidays, plans, streak and freezes, pace) with Catch2 tests and golden-master parity against the web engine; English first-run welcome flow that stores the learner profile and their reason for learning in SQLite; Today screen
- [x] Phase 3: the daily plan end to end: lessons, spaced-repetition review, the eight focus trainers, phase checkpoints and the B1 mock, with progress stored in SQLite, answer checking and text-to-speech with the system voices (Qt TextToSpeech)
- [x] Built-in neural Spanish voice (Supertonic 2 on onnxruntime since 0.3.4, offline, the same on Windows and macOS) with a voice per dialogue character; symbols and numbers read as words
- [x] Guiding coach with a reflection every three sessions; mixed practice leans towards what the learner enjoys
- [x] Practice polish (0.3.5): misses come back in a second-try round at the end worth more XP; the streak's own flame icon or any emoji the learner picks
- [x] Settings (0.3.6): day and night mode, the learner's own voice out of six (dialogue characters keep theirs) and its normal and slow speed; a less robotic voice
- [x] Phase 4 (0.4.0): browsing screens (practice hub, words with search, progress with achievements and signs of B1); immersion log with listening milestones; a backup compatible with the web app's
- [x] Voice fixes (0.4.5): every voice at the same pace and loudness, pauses between sentences, a noise seed per voice, M2 as narrator and learner
- [x] First public release (0.4.6 in the repository, 0.4.7 as downloads): fixes from an outside review of saving, restoring and backups; a download button in Settings with a reminder to check for updates every 15 days; releases with `SHA256SUMS.txt` built from a version tag
- [ ] Web app: offer the browser's Spanish language pack so *Check words* can run on the device
- [ ] Desktop app: tell the learner when LearnQuick is already open instead of closing without a word; sharper icons at small sizes
- [ ] Phase 5: on-device speech recognition (whisper.cpp) and conversation practice (llama.cpp) that uses the learner's stated reason as context; microphone usage descriptions
- [ ] Hands-on testing on an Apple Silicon Mac (interface, voices, microphone, Metal performance)

## Milestone 3 — Language-learning depth

- [ ] Content quality review by a native Spanish teacher
- [ ] More listening material inside the app (graded audio stories)
- [ ] Adaptive review of weak units

## Milestone 4 — Release preparation

- [ ] Accessibility audit and localization review
- [ ] Privacy review
- [ ] Testing on supported macOS versions and architectures
- [ ] Code signing for Windows and Apple (Developer ID and notarization); until then releases are unsigned, under the rules in [SECURITY.md](../SECURITY.md#unsigned-downloads)
- [ ] Release notes for each version

This roadmap is provisional. No release dates or proficiency guarantees are implied.
