# Release notes template

Copy this into every public release, replacing `X.Y.Z` and the first section. [SECURITY.md](../SECURITY.md#unsigned-downloads) explains why these parts are required.

---

## LearnQuick X.Y.Z

What changed in this version.

### Download

- **Windows:** `LearnQuick-windows.zip`
- **macOS:** `LearnQuick-macos.zip`

This page is the only official place to download LearnQuick. Your progress carries over when you update.

### Check for updates every 15 days

LearnQuick never connects to the internet, so it cannot update itself or tell you that a new version exists. About every 15 days, come back to this page, or open **Settings → Updates → Download latest version** in the app: if the version here is newer than yours, download it. Your progress carries over.

### Check your download

`SHA256SUMS.txt` holds the fingerprint of each file. Run this in the folder that holds your download and compare the result with that file:

- **Windows** (PowerShell): `Get-FileHash LearnQuick-windows.zip -Algorithm SHA256`
- **macOS** (Terminal): `shasum -a 256 LearnQuick-macos.zip`

If the fingerprints are not identical, delete the file and download it again. Windows shows its one in capital letters, which makes no difference.

### Why Windows and macOS warn you

LearnQuick is not code-signed yet, so Windows SmartScreen and macOS Gatekeeper cannot tell who made it and warn you the first time you open it. The warning is about the missing signature: it does not mean that anything harmful was found. The installation guide in this repository (`docs/INSTALL.md`) has the steps to get past it.
