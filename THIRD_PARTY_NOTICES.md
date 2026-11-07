# Third-party notices

LearnQuick's own code is under the MIT License ([LICENSE](LICENSE)). This file lists the third-party software that the **desktop app** ships with, and the terms it comes under. The texts of every license named here are in [LICENSES/](LICENSES/), and the desktop app carries this file, `LICENSE` and that folder in its `licenses` folder.

The **web app** (`app/`, `LearnQuick.html`) uses no third-party code.

Not shipped: Catch2 (used only to build the tests) and espeak-ng (`espeakng-loader`, used only by `desktop/tools/phoneme_reference.py` to check the app's own phonemizer).

## Qt

LearnQuick uses the Qt framework, version 6.10.3 in the Windows build and 6.12.0 in the macOS build, under the **GNU Lesser General Public License version 3** ([LGPL-3.0-only](LICENSES/LGPL-3.0-only.txt), which builds on the [GPL-3.0](LICENSES/GPL-3.0-only.txt)). Qt is © The Qt Company Ltd. and other contributors.

- **Dynamically linked and unmodified.** Qt's libraries ship as separate files that you may replace with your own build of the same Qt version:
  - Windows: the `Qt6*.dll` files and the `plugins` and `qml` folders next to `LearnQuick.exe`;
  - macOS: `LearnQuick.app/Contents/Frameworks` and `Contents/PlugIns`. Replacing them invalidates the app's signature; sign it again with `codesign --force --deep --sign - LearnQuick.app`.
- **Source code:** the complete source of each version is published by The Qt Company:
  - 6.10.3: <https://download.qt.io/official_releases/qt/6.10/6.10.3/single/qt-everywhere-src-6.10.3.tar.xz>
  - 6.12.0: <https://download.qt.io/official_releases/qt/6.12/6.12.0/single/qt-everywhere-src-6.12.0.tar.xz>
- Nothing in LearnQuick's terms forbids modifying the app for your own use, or reverse engineering it to debug such modifications.

### Third-party code inside Qt

The Qt libraries the app ships contain the components below. The list comes from Qt's own software bill of materials (SBOM) for Qt 6.10.3; the macOS build uses Qt 6.12.0, whose versions may be newer. Some are used on one system only: `wintab`, `d3d12memoryallocator` and `rhi-miniengine-d3d12-mipmap` on Windows, the `libdbus-1` headers (in QtDBus) on macOS. Where a component offers a choice of licenses, the one shown is the one LearnQuick takes.

| Component | Version | License | Copyright |
|---|---|---|---|
| aglfn | 1.7 | BSD-3-Clause | Copyright 2002, 2003, 2005, 2006, 2008, 2010, 2015 Adobe Systems |
| blake2 | ed1974ea8343 | CC0-1.0 | Copyright 2012, Samuel Neves <sneves@dei.uc.pt> |
| d3d12memoryallocator | f128d39b7a95 | MIT | Copyright (c) 2019-2022 Advanced Micro Devices, Inc. All rights reserved. |
| doubleconversion | 3.4.0 | BSD-3-Clause | Copyright 2006-2012, the V8 project authors |
| dr_libs | 0.14.5 | MIT-0 | No copyright notice (public domain or MIT-0) |
| easing | unknown | BSD-3-Clause | Copyright (c) 2001 Robert Penner |
| eigen | 3.4.0 | MPL-2.0 AND BSD-3-Clause | Copyright (c) 2012 Alexey Korepanov <kaikaikai@yandex.ru> Copyright (c) 2020 Antonio Sanchez <cantonios@google.com> Copyright (c) 2020 Arm Limited and… |
| emoji-segmenter | 0.4.0 | Apache-2.0 | Copyright 2019 Google LLC |
| ffmpeg-boost | 7.1.3 | BSL-1.0 | (C) Copyright John Maddock 2006. Copyright (c) 2006 Xiaogang Zhang |
| ffmpeg-libjpeg | 7.1.3 | IJG | Copyright (C) 1991-2020, Thomas G. Lane, Guido Vollbeding. |
| ffmpeg-zlib | 7.1.3 | Zlib | Copyright (C) 1995-2022 Jean-loup Gailly and Mark Adler. |
| FreeType | 2.14.3 | FTL | Copyright (c) 2007-2014 Adobe Systems Incorporated Copyright (c) 2004-2026 Albert Chin-A-Young Copyright (c) 2018-2026 Armin Hasitzka, David Turner, Robert… |
| freetype-bdf | 2.14.3 | MIT | Copyright (c) 2000 Computing Research Labs, New Mexico State University Copyright (c) 2001-2014 Francesco Zappa Nardelli |
| freetype-pcf | 2.14.3 | MIT AND MIT-open-group | Copyright (c) 2001, 2012 David Turner, Robert Wilhelm, and Werner Lemberg Copyright (c) 2000-2014 Francesco Zappa Nardelli Copyright (c) 1990, 1994, 1998… |
| freetype-zlib | 2.14.3 | Zlib | Copyright (C) 1995-2024 Jean-loup Gailly and Mark Adler |
| grayraster | unknown | FTL | Copyright 2000-2016 by David Turner, Robert Wilhelm, and Werner Lemberg. |
| HarfBuzz | 13.2.1 | MIT | Copyright © 2010-2022 Google, Inc. Copyright © 2015-2020 Ebrahim Byagowi Copyright © 2019,2020 Facebook, Inc. Copyright © 2012,2015 Mozilla Foundation… |
| icc-srgb-color-profile | unknown | LicenseRef-ICC-License | Copyright International Color Consortium, 2015 |
| libdbus-1-headers | 1.13.12 | AFL-2.1 | Copyright (C) 2002, 2003 CodeFactory AB Copyright (C) 2004, 2005 Red Hat, Inc. |
| libjpeg-turbo | 3.1.4 | IJG AND BSD-3-Clause | Copyright (C) 2009-2026 D. R. Commander Copyright (C) 2015-2016, 2018, 2022 Matthieu Darbois Copyright (C) 2019-2021 Arm Limited Copyright (C) 2015, 2020… |
| libpng | 1.6.56 | Libpng AND libpng-2.0 | Copyright (c) 1995-2026 The PNG Reference Library Authors Copyright (c) 2000-2026 Cosmin Truta Copyright (c) 1998-2018 Glenn Randers-Pehrson Copyright (c)… |
| libpsl | 664f3dc85259 | BSD-3-Clause | Copyright 2014-2016 The Chromium Authors. All rights reserved. |
| masm | unknown | BSD-2-Clause | Copyright (C) 2003-2018 Apple Inc. All rights reserved. Copyright (C) 2007 Justin Haygood (jhaygood@reaktix.com) Copyright (C) 2007-2009 Torch Mobile, Inc.… |
| md4 | unknown | CC0-1.0 | Written by Alexander Peslyak - better known as Solar Designer <solar@openwall.com> - in 2001, and placed in the public domain. There's absolutely no warranty. |
| md4c | 0.5.2 | MIT | Copyright © 2016-2024 Martin Mitáš |
| md5 | unknown | CC0-1.0 | Written by Colin Plumb in 1993, no copyright is claimed. Ian Jackson <ian@chiark.greenend.org.uk>. |
| opengl-es2-headers | Revision 276 | MIT | Copyright (c) 2013-2014 The Khronos Group Inc. |
| opengl-headers | Revision 276 | MIT | Copyright (c) 2013-2014 The Khronos Group Inc. |
| PCRE2 | 10.47 | LicenseRef-BSD-3-Clause-with-PCRE2-Binary-Like-Packages-Exception | Copyright (c) 1997-2007 University of Cambridge Copyright (c) 2007-2024 Philip Hazel Copyright (c) 2010-2024 Zoltan Herczeg |
| pcre2-sljit | 10.47 | BSD-2-Clause | Copyright Zoltan Herczeg |
| pffft | 02fe7715a5bf | BSD-3-Clause | Copyright (c) 2013 Julien Pommier <pommier@modartt.com> Copyright (c) 2004 the University Corporation for Atmospheric Research 'UCAR' |
| psl-data | 2026-01-16_1 | MPL-2.0 | The list was originally provided by Jo Hermans <jo.hermans@gmail.com>. It is now maintained on github (https://github.com/publicsuffix/list). |
| Resonance Audio | e225aedb5ec7 | Apache-2.0 | Copyright (c) 2015, 2017, 2018 Google Inc. |
| rfc6234 | unknown | BSD-3-Clause | Copyright (c) 2011 IETF Trust and the persons identified as authors of the code. |
| rhi-miniengine-d3d12-mipmap | 0aa79bad7899 | MIT | Copyright (c) 2015 Microsoft |
| sha1 | unknown | LicenseRef-SHA1-Public-Domain | Copyright (C) Dominik Reichl <dominik.reichl@t-online.de> Copyright (C) 2016 The Qt Company Ltd |
| sha3_endian | 1.0.0 | BSD-2-Clause | Copyright (c) 1998-2013, Brian Gladman, Worcester, UK. All rights reserved. |
| sha3_keccak | 3.2 | CC0-1.0 | Guido Bertoni, Joan Daemen, Michaël Peeters and Gilles Van Assche. To the extent possible under law, the implementers have waived all copyright and related… |
| shadow_angular_material | unknown | MIT | Copyright (c) 2014-2016 Google, Inc |
| signalsmith-stretch | 1.0.0 | MIT | Copyright (c) 2022 Geraint Luff / Signalsmith Audio Ltd. |
| siphash | unknown | CC0-1.0 | Copyright (C) 2012-2014 Jean-Philippe Aumasson Copyright (C) 2012-2014 Daniel J. Bernstein <djb@cr.yp.to> Copyright (C) 2016 Intel Corporation |
| smooth-scaling-algorithm | unknown | BSD-2-Clause AND Imlib2 | Copyright (C) 2004, 2005 Daniel M. Duley. (C) Carsten Haitzler and various contributors. (C) Willem Monsuwe <willem@stack.nl> |
| sqlite | 3.51.3 | blessing | The authors disclaim copyright to the source code. However, a license can be obtained if needed. |
| tika-mimetypes | 408c26e1e03e | Apache-2.0 | Copyright 2026 The Apache Software Foundation |
| tinycbor | 7.0 | MIT | Copyright (C) 2015-2025 Intel Corporation |
| tlexpected | 41d3e1f48d68 | CC0-1.0 | To the extent possible under law, Sy Brand has waived all copyright and related or neighboring rights to the expected library. This work is published from:… |
| TLSF | v3.1 | BSD-3-Clause | Copyright (c) 2006-2016, Matthew Conte |
| unicode-character-database | 36 | Unicode-3.0 | Copyright (C) 1991-2025 Unicode, Inc. |
| unicode-cldr | v48.1 | Unicode-3.0 | Copyright (C) 2004-2025 Unicode, Inc. |
| vulkan-xml-spec | 1.4.308 | MIT | Copyright (c) 2015-2025 The Khronos Group Inc. |
| vulkanmemoryallocator | 3.2.1 | MIT | Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved. |
| webgradients | unknown | MIT | Copyright (c) 2017 itmeo |
| wintab | unknown | LicenseRef-Lcs-Telegraphics | Copyright 1991-1998 by LCS/Telegraphics. |
| xserverhelper | unknown | X11 AND HPND | Copyright (c) 1987, 1988 X Consortium Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts. |
| xsvg | unknown | HPND-sell-variant | Copyright 2002 USC/Information Sciences Institute |
| yoga | 2.0.1 | MIT | Copyright (c) Facebook, Inc. and its affiliates. Copyright (c) Meta Platforms, Inc. and affiliates. |
| zlib | 1.3.2 | Zlib | (C) 1995-2026 Jean-loup Gailly and Mark Adler |

Portions of this software are copyright © The FreeType Project (www.freetype.org). All rights reserved.

## FFmpeg

Qt Multimedia plays audio through FFmpeg, which ships as separate shared libraries (`avcodec`, `avformat`, `avutil`, `swresample`, `swscale`) under the **GNU Lesser General Public License version 2.1 or later** ([LGPL-2.1-or-later](LICENSES/LGPL-2.1-or-later.txt)). FFmpeg is © the FFmpeg developers.

- **Versions:** 7.1.3 on Windows, 9.0.1 on macOS, built by The Qt Company and unmodified by LearnQuick.
- **Configuration:** built without `--enable-gpl`, `--enable-nonfree` or `--enable-version3`; the libraries report "LGPL version 2.1 or later". The Windows build was configured with `--disable-programs --disable-doc --disable-debug --enable-network --disable-lzma --enable-pic --disable-vulkan --disable-v4l2-m2m --disable-decoder=truemotion1 --enable-zlib --toolchain=msvc --enable-shared --disable-static`, and links zlib 1.3.1 ([Zlib](LICENSES/Zlib.txt), © Jean-loup Gailly and Mark Adler).
- **Source code:** <https://ffmpeg.org/releases/ffmpeg-7.1.3.tar.xz> and <https://ffmpeg.org/releases/ffmpeg-9.0.1.tar.xz>.
- You may replace these libraries with your own build of the same version: the `av*.dll` and `sw*.dll` files on Windows, the `libav*` and `libsw*` files in `Contents/Frameworks` on macOS.

## ONNX Runtime 1.23.2

Runs the built-in voice. MIT License, © Microsoft Corporation. Its license and the notices of the components it includes (among them Eigen, under the [MPL-2.0](LICENSES/MPL-2.0.txt), whose source is at <https://gitlab.com/libeigen/eigen>) ship with the app in `voices/onnxruntime` (`LICENSE`, `ThirdPartyNotices.txt`). Source: <https://github.com/microsoft/onnxruntime/tree/v1.23.2>.

## Supertonic 2 voice model

The built-in Spanish voice is the Supertonic 2 model by Supertone Inc., distributed unmodified under the **BigScience Open RAIL-M License**. Its full text ships with the app as `voices/LICENSE`.

**The model's use-based restrictions apply to everyone who uses LearnQuick's built-in voice.** By using the voice you agree not to use the model in any of the ways listed in paragraph 5 and Attachment A of that license (for example: against the law; to harm minors; to generate or spread false information meant to harm others; to impersonate others without their consent; or to present its output without saying it is machine generated). Details: [desktop/voice/THIRD_PARTY_NOTICES.md](desktop/voice/THIRD_PARTY_NOTICES.md).

LearnQuick's own code that runs the model follows the `supertonic` Python package 1.3.1: MIT License, Copyright (c) 2025 Supertone Inc.

## nlohmann/json 3.12.0

Compiled into the app to read the course and backups. MIT License, Copyright (c) 2013-2025 Niels Lohmann.

## Windows only: Microsoft and Mesa components

Qt's deployment tool adds these files to the Windows build:

- `d3dcompiler_47.dll`, `dxcompiler.dll` and `dxil.dll`: Microsoft DirectX shader compilers, redistributed under the Microsoft Windows SDK license terms.
- `vc_redist.x64.exe`: the Microsoft Visual C++ Redistributable, under its own Microsoft license terms.
- `opengl32sw.dll`: a software OpenGL renderer (Mesa 11.2.2, llvmpipe) built by The Qt Company, used only when the computer has no working graphics driver. Mesa is under the [MIT](LICENSES/MIT.txt) License; it includes LLVM, under the [University of Illinois/NCSA](LICENSES/NCSA.txt) license.
- `icuuc.dll`: a copy of Windows' own Unicode library (© Microsoft Corporation), part of Windows 10 version 1903 and later.
