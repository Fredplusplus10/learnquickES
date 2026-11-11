# Security policy

## Status

LearnQuick is a local, single-learner application. Neither app has a server, an account or any analytics, and neither sends anything over the network. It is not meant for confidential data.

**What each app keeps:**

- **Web app** (`LearnQuick.html`): progress lives in the browser's local storage on the learner's computer, and in the backup files the learner downloads.
- **Desktop app** (Windows and macOS): progress lives in one SQLite file in the user's application-data folder:
  - Windows: `%APPDATA%\LearnQuick\LearnQuick\learnquick.sqlite3`;
  - macOS: `~/Library/Application Support/LearnQuick/LearnQuick/learnquick.sqlite3`.

  It holds the learner's name, their reason for learning, settings, answers, study minutes, immersion notes and saved writing. Backups are files the learner saves.

**What the desktop app reads:**

- Only files the learner chooses or drops onto it: a backup, or a viewing history.
- One exception: the Netflix viewing history. The app looks in the *Downloads* folder for a file named `NetflixViewingHistory*.csv`, or `netflix-report*/…/CONTENT_INTERACTION/ViewingActivity.csv`, saved in the last two weeks.
  - On Windows it looks when Immersion opens and when the app comes back to the front.
  - On macOS it looks only after the learner presses *Look in Downloads*, and macOS asks for permission the first time.
  - The file is read on the computer and only what was chosen is kept: titles, dates and minutes. See [docs/NETFLIX-GUIDE.md](docs/NETFLIX-GUIDE.md).
- It never asks for a streaming-service password, and it has no code that opens a network connection.
- Links to recommended resources, and *Download latest version* in Settings, open in the learner's own browser. The app itself makes no request.

## What is protected, and what is not

**Protected:**

- **SQL:** database access uses bound parameters; table names in queries are constants.
- **Saving, restoring and migrating** each run in one transaction, so a failure leaves the previous data as it was.
- **A copy before each update:** before the desktop app moves the database to a new schema, it saves a full copy next to it (`learnquick.sqlite3.v<N>.bak`). If the copy cannot be written, the database is left untouched.
- **Opening a newer database:** a database written by a newer version of the app is refused, not rewritten.
- **Imported files are capped:**
  - backups at 8 MB, viewing histories at 32 MB;
  - each field gets the type and the limits the screens use.

  A tampered backup cannot put markup on a page: the web app turns every restored field back into a number, a date or plain text, and the desktop app shows learner text as plain text.
- **Certificates:** an issued *Certificate of Readiness* and the assessment attempt it rests on cannot be changed or deleted through the app (database triggers). The level assessment is marked in C++, its right answers never reach the screen, and a finished attempt is stored only when the assessment is open that day.

**Not protected, by design:**

- **The learner's own data:** whoever can use the computer can open the database or edit a backup. Progress, the streak and achievements are trusted to the learner, as in any offline app.
- **Certificates of Readiness** are LearnQuick's own records, not CEFR or DELE qualifications. They carry no cryptographic signature, and a third party cannot verify them. The level assessment covers reading, listening, and grammar and vocabulary, not speaking or writing, and its questions ship inside the app, where a determined learner could read them.
- **Desktop builds** are not code-signed yet. Windows SmartScreen and macOS Gatekeeper warn about them (see [Unsigned downloads](#unsigned-downloads)).

## Unsigned downloads

LearnQuick's first public releases are **not code-signed**. This is a deliberate, temporary decision: signing needs a Windows certificate and an Apple Developer ID, and it is pending (see the [roadmap](docs/ROADMAP.md)). To make up for it, every public release must follow these rules:

- **One official source.** The only official downloads are on the *Releases* page of the public repository. A copy from anywhere else is not official.
- **A published fingerprint.** Each release carries a `SHA256SUMS.txt` and explains how to check it, with one command for Windows and another for macOS (see [Check your download](docs/INSTALL.md#check-your-download)).
- **The warnings explained.** Each release says that SmartScreen and Gatekeeper warn because the app is not signed, not because anything harmful was found. [docs/RELEASE_NOTES_TEMPLATE.md](docs/RELEASE_NOTES_TEMPLATE.md) holds that text.

A matching fingerprint shows that the file is the one published and that it arrived intact. It does not show who built it: someone who took over the repository could replace the files and their fingerprints together. Only code signing closes that gap.

The release workflow follows these rules by itself: from a version tag it builds both downloads, writes `SHA256SUMS.txt` and fills the release notes from that template. The release stays a draft until the maintainer has checked it.

## The repository

- **No secrets to steal.** The apps use no keys, tokens or passwords, and the repository's GitHub Actions have no secrets. The workflows' token is read-only, except in the one job that creates a release.
- **No learner data.** Databases, backups and viewing histories stay on the learner's computer. `.gitignore` leaves them out, and `scripts/validate_repository.py` fails when a tracked file looks like a key, a personal email address or a learner's file.
- **Commits** use the maintainer's GitHub noreply address, and contributors should use theirs (see [CONTRIBUTING.md](CONTRIBUTING.md)).

## Reporting vulnerabilities

Use GitHub's private vulnerability reporting if the maintainer enables it. Otherwise, contact the maintainer privately through a channel on their GitHub profile. Do not publish exploitable details in a public issue.

## Principles for future work

- No mandatory server-side learner profiles or cloud synchronization.
- Database access through parameterized queries and explicit transactions.
- Microphone and folder permissions requested only for features the learner starts.
- Explicit retention and export/deletion policies for study events and recordings.
- Dependency review before each public release ([docs/DEPENDENCY_REVIEW.md](docs/DEPENDENCY_REVIEW.md) holds the latest).
- Code signing for Windows and Apple, which will replace the measures in [Unsigned downloads](#unsigned-downloads).
