# Contributing to LearnQuick

## Scope

LearnQuick is a working local web application (`app/`) for a single learner. The native desktop app (C++20 + Qt 6) is being built in `desktop/`.

## Contribution process

1. Review the README, `docs/ARCHITECTURE.md` and `docs/STATUS.md`.
2. Open a focused issue for a feature proposal or defect.
3. Keep changes small, with one technical concern per pull request.
4. Describe how the behavior was validated and distinguish mocked or demo data from real behavior.
5. Do not introduce network telemetry, remote fonts, tracking SDKs, CDN dependencies or proprietary assets without explicit approval.

## Code quality expectations

- Keep the app dependency-free and runnable from `file://`: classic scripts, no ES modules, no `fetch`.
- Keep scheduling, streak, SRS and conjugation logic in `app/js/core/` as pure, deterministic functions, with tests.
- In `desktop/`: C++20, no compiler warnings, logic in C++ (never in QML), and `ctest` (smoke test) must pass. Build steps: [desktop/README.md](desktop/README.md).
- Use semantic, keyboard-accessible controls. Never use colour as the only signal.
- Content changes (lessons, trainers) must keep `tests/content.test.js` passing; every exercise must be solvable.
- Avoid undocumented side effects and unnecessary runtime dependencies.
- Keep all design assets editable and licensed appropriately.
- Do not claim a learning outcome unless independently supported.

## Keep private things out

- **Commit with your GitHub noreply address**, not a personal one. Find it under GitHub → Settings → Emails, then run `git config user.email "ID+username@users.noreply.github.com"` in your clone.
- **Never commit keys, tokens or passwords.** The app needs none: it has no server, no account and no API. If one slips into a commit, revoke it first; deleting the file does not remove it from the history.
- **Never commit a learner's data:** the database (`learnquick.sqlite3`), backups (`learnquick-backup-*.json`) or streaming histories (`NetflixViewingHistory*.csv`, `netflix-report*/`). Use the demo learner (`--demo`, `?demo`) to test, and fake data in tests.

`.gitignore` leaves these files out, and `scripts/validate_repository.py` fails when a tracked file holds a key, a personal email address or a learner's data file. Pull requests run it too.

## Verification

```bash
python3 scripts/validate_repository.py
node --test tests/core.test.js tests/content.test.js
for f in app/js/*.js app/js/*/*.js; do node --check "$f"; done

# Native app (needs Qt 6.8+ and a C++20 compiler)
cmake -S desktop -B build/desktop -DCMAKE_BUILD_TYPE=Debug
cmake --build build/desktop
ctest --test-dir build/desktop --output-on-failure
```

Then open `app/index.html?demo` and check the affected screens by hand.

## License of contributions

LearnQuick's code is under the MIT License ([LICENSE](LICENSE)). By contributing code you agree that it is released under the same license. The graphic assets (illustrations, logos, app icons, interface icons and certificate designs) are all rights reserved and are not open to outside changes. Never add third-party code, fonts or assets without listing them in [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), with a license compatible with MIT.
