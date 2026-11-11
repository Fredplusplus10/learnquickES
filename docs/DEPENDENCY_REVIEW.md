# Dependency review

[SECURITY.md](../SECURITY.md) asks for this review before each public release. This one is for **LearnQuick 1.4.6**, done on **2026-11-11**.

**Result: nothing blocks the release.** Published advisories exist for the versions of Qt, FFmpeg, SQLite and ONNX Runtime that the desktop app ships. Each of them needs a crafted file to reach the affected code, and the app gives that code only files that come inside the app. Three updates are due and are listed under [Follow-up](#follow-up).

## What was checked, and how

- **Scope:** everything the desktop app ships, what its build downloads, and what the workflows run. The web app (`app/`, `LearnQuick.html`) has no third-party code.
- **Advisories:** [Qt's list of known vulnerabilities](https://wiki.qt.io/List_of_known_vulnerabilities_in_Qt_products) and [security advisories](https://www.qt.io/blog/tag/security), GitHub's advisory database and the security advisories of each library's repository, and a web search for each pinned version.
- **Exposure:** for every advisory that names a version in use, whether the app passes outside data to the affected code. The only outside data the desktop app reads are a backup (JSON, read by nlohmann/json), a viewing history (CSV, read by the app's own parser) and its own SQLite database.
- **Not done:** no binary or SBOM scan, and the third-party code inside Qt (FreeType, HarfBuzz, libpng, libjpeg-turbo, PCRE2 and the rest in [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md)) was checked only through Qt's own advisories.

## Shipped with the desktop app

| Dependency | Version | Pinned by | License | Advisories that name this version | Reached by outside data? |
|---|---|---|---|---|---|
| Qt (Windows) | 6.10.3 | version | LGPL-3.0-only | 28 (see [Qt 6.10.3](#qt-6103-on-windows)) | No |
| Qt (macOS) | 6.12.0 | version | LGPL-3.0-only | 5 (see [Qt 6.12.0](#qt-6120-on-macos)) | No |
| FFmpeg, inside Qt Multimedia | 7.1.3 (Windows), 9.0.1 (macOS) | Qt's version | LGPL-2.1-or-later | CVE-2026-8461 on Windows | No: the app decodes no media file |
| SQLite, inside Qt SQL | 3.51.3 | Qt's version | public domain | A heap overflow in the FTS5 extension is reported for versions before 3.53.2 | No: the app does not use FTS5 |
| ONNX Runtime | 1.23.2 | version and SHA-256 | MIT | Fixes after 1.23.2 (see [ONNX Runtime](#onnx-runtime-1232)) | No: only the bundled model is loaded |
| Supertonic 2 voice model | commit `75e6727` | commit and SHA-256 of each file | BigScience Open RAIL-M | None found | Data, not code |
| nlohmann/json | 3.12.0 | version and SHA-256 | MIT | None found; it is the latest release | Yes, by design: it reads backups (see below) |

**nlohmann/json reads outside data.** A backup comes from the learner or from someone else. The app refuses a file over 8 MB before parsing, catches every parsing error, and since 1.4.6 refuses a backup of the wrong shape or with a number that is negative or does not fit.

## Used to build and test, not shipped

| Dependency | Version | Pinned by | Notes |
|---|---|---|---|
| Catch2 | 3.16.1 | version and SHA-256 | Tests only. Latest release; no advisory found |
| GitHub Actions (`checkout`, `setup-python`, `setup-node`, `upload-artifact`, `download-artifact`, `install-qt-action`) | see the workflows | commit SHA | The workflows' token is read-only, except in the job that publishes a release |
| `espeakng-loader` | 0.2.4 | version | Used by one check in CI; never part of the app |
| Qt, as downloaded in CI | 6.10.3, 6.12.0 | version | `install-qt-action` downloads it from Qt's servers; the version is fixed, the files are not pinned by hash |

## Qt 6.10.3 on Windows

Qt's list names 28 advisories whose range includes 6.10.3. The app links Core, Gui, Multimedia, Qml, Quick, Sql, Svg and TextToSpeech.

**In modules the app uses:**

| Advisory | Where | Why the app is not exposed |
|---|---|---|
| CVE-2026-6210, CVE-2026-8168 | Qt SVG: marker handling, deep nesting | The only SVG files drawn are the app's own icons, illustrations and certificate template. The learner's name enters the certificate through `QXmlStreamWriter`, which escapes it |
| CVE-2026-79616 | Qt Quick: `PathSvg.path`, `Context2D.path` | The paths are constants in `Icon.qml` and `StreakIcon.qml`; the one `Canvas` draws from numbers |
| CVE-2026-105923 | Qt Quick: `VectorImage` | Not used |
| CVE-2026-16762, CVE-2026-78253 | Qt Core: `QXmlStreamReader` | It reads only the certificate template that ships with the app |
| CVE-2026-101201, CVE-2026-107157, CVE-2026-105924 | Qt Core: JSON and CBOR | The app reads its JSON with nlohmann/json and uses no CBOR |
| CVE-2026-8461 | Qt Multimedia: FFmpeg 7.1.3 | The app plays the sound its own voice engine makes, as raw samples through `QAudioSink`. No media file is opened |

**In modules the app does not use:** Qt Network (CVE-2026-105946, -95620, -96432, -76151), Qt XML (CVE-2026-82292, -19248, -15037), Qt5Compat (CVE-2026-107479, -107590, -107472, -100140, -9499), Qt NFC, MQTT, CoAP, HTTP Server, WebSockets, 3D and the VNC server. The app has no code that opens a network connection.

Fixed in Qt 6.11.3 and 6.12.1. The Windows build stays on 6.10.3 because the tool that installs Qt in CI cannot download 6.11 or later for Windows yet (see `.github/workflows/desktop.yml`).

## Qt 6.12.0 on macOS

Five advisories include 6.12.0: HTTP Server (CVE-2026-110944), WebSockets (CVE-2026-114366), Qt 3D (CVE-2026-111487), Qt5Compat (CVE-2026-100140) and CBOR to JSON (CVE-2026-105924). The app uses none of them. Fixed in 6.12.1.

## ONNX Runtime 1.23.2

The latest release is 1.31.1. The app stays on 1.23.2 because it is the last release with a universal (Apple Silicon and Intel) macOS build.

Fixes published after 1.23.2 include a path traversal through a model's external data (fixed in 1.24.1), an out-of-bounds read in `ArrayFeatureExtractor` and a crash in the transpose optimizer. Each needs a crafted model. LearnQuick loads four model files and nothing else: they are downloaded when the app is built, checked against a SHA-256 written in `desktop/voice/CMakeLists.txt`, and shipped inside the app. The learner cannot give the app a model.

## Licenses

What ships matches [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md): Qt and FFmpeg as separate, replaceable libraries under the LGPL; ONNX Runtime and nlohmann/json under the MIT License; the voice model under the BigScience Open RAIL-M License, whose use restrictions travel with it. No dependency has changed since those notices were written.

## Follow-up

- **Qt on Windows:** move to 6.11.3 or later, or 6.12.1 or later, as soon as CI can install it.
- **Qt on macOS:** move from 6.12.0 to 6.12.1 or later.
- **ONNX Runtime:** look again for a newer release with a universal macOS build, or build it for both architectures.

None of these changes what a learner is exposed to today. They matter the day the app starts opening files it did not bring: the review must be repeated before any feature that draws a learner's SVG, plays a learner's media file, loads another model or opens a network connection.
