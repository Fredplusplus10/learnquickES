# Licensing

## LearnQuick's code: MIT

The code is under the MIT License ([LICENSE](../LICENSE)): anyone may use, copy, change and distribute it, also commercially, as long as the copyright notice and the license travel with it.

## Graphic assets: all rights reserved

The MIT License does not cover the illustrations, logos, app icons, interface icons and certificate designs. The interface icons are LearnQuick's own: none matches the same-named icons of Lucide, Feather or Tabler. They stay all rights reserved: they may be shown as part of LearnQuick, but not reused elsewhere without permission. The folders are listed in the README, under *License*.

## Third-party software

Everything the desktop app ships from others, with its version, license and where to get its source, is in [THIRD_PARTY_NOTICES.md](../THIRD_PARTY_NOTICES.md). The license texts are in the `LICENSES/` folder. The desktop app carries `LICENSE`, `THIRD_PARTY_NOTICES.md` and `LICENSES/` in its `licenses` folder:

- Windows: `licenses/` next to `LearnQuick.exe`;
- macOS: `LearnQuick.app/Contents/Resources/licenses/`.

What each license asks of LearnQuick:

- **Qt (LGPL-3.0) and FFmpeg (LGPL-2.1 or later):**
  - they stay separate shared libraries that users can replace;
  - they are not modified;
  - their license texts ship with the app;
  - the notices say where their source is.
  
  Both were checked in the built apps: the Windows `LearnQuick.exe` imports `Qt6*.dll`; the macOS app loads Qt frameworks and FFmpeg dylibs through `@rpath`. FFmpeg was built without `--enable-gpl` or `--enable-nonfree`. A static build of Qt, or an FFmpeg built with those options, would change these obligations.
- **The Supertonic 2 voice (BigScience Open RAIL-M):**
  - its use restrictions (paragraph 5 and Attachment A) must be part of the terms under which the app is distributed, and users must be told about them; `THIRD_PARTY_NOTICES.md` and the README do this;
  - every recipient gets a copy of the license, which ships as `voices/LICENSE`.
- **ONNX Runtime and nlohmann/json (MIT)** and the permissive components inside Qt: their copyright notices and licenses ship with the app.

None of these conflicts with releasing LearnQuick's own code under MIT.

## Updating the notices

When Qt, FFmpeg, ONNX Runtime or the voice changes version, update `THIRD_PARTY_NOTICES.md`. The table of components inside Qt comes from Qt's SBOM files (`<Qt prefix>/sbom/*.spdx.json`).
