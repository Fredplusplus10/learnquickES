# Installing LearnQuick

LearnQuick comes in two forms:

- **The desktop app**, for Windows and macOS. It has the full course, a built-in Spanish voice and keeps your progress in a file on your computer.
- **The web app**, which runs in any browser from a double-click, with nothing to install (see [the web app](#the-web-app) below).

Both work offline, need no account and never send anything over the internet.

## What you need

| | Windows | macOS |
|---|---|---|
| System | Windows 10 or 11, 64-bit | macOS 14.4 Sonoma or later, on Apple Silicon or Intel (one app for both) |
| Free space | About 450 MB | About 450 MB |
| Internet | Only to download the app | Only to download the app |

## Download

1. Open the latest release: <https://github.com/Fredplusplus10/learnquickES/releases/latest>. If you already have LearnQuick, **Settings → Download latest version** opens the same page.
2. Under **Assets**, download **`LearnQuick-windows.zip`** or **`LearnQuick-macos.zip`**. You do not need a GitHub account.

That **Releases** page is the only official place to download LearnQuick. A copy from anywhere else is not official.

### Check your download

Each release includes a `SHA256SUMS.txt` file with the fingerprint of every download. To check yours, run this in the folder that holds it and compare the result with that file:

- **Windows** (PowerShell): `Get-FileHash LearnQuick-windows.zip -Algorithm SHA256`
- **macOS** (Terminal): `shasum -a 256 LearnQuick-macos.zip`

The two fingerprints must be identical; Windows shows its one in capital letters, which makes no difference. If they are not, delete the file, download it again and do not open it until they match.

### Why Windows and macOS warn you

LearnQuick is not code-signed yet, so Windows SmartScreen and macOS Gatekeeper cannot tell who made it and warn you the first time you open it. The warning is about the missing signature: it does not mean that anything harmful was found. The steps to get past it are under [Windows](#windows) and [macOS](#macos) below.

## Windows

### Install

1. **Extract the whole zip.** Right-click `LearnQuick-windows.zip` → **Extract All…** and choose a folder that will stay put, for example `C:\Users\<you>\Apps\LearnQuick`. Do not run the app from inside the zip: it needs the files around it.
2. Open the `bin` folder and double-click **LearnQuick.exe**.
3. **The first time**, Windows may show a blue window, *Windows protected your PC*. LearnQuick is not signed with a paid certificate yet. Click **More info → Run anyway**. Each new version asks once.

To find it again easily, right-click `LearnQuick.exe` → **Show more options → Send to → Desktop (create shortcut)**, or **Pin to Start**.

### If it does not start

- **Nothing happens, or "the application was unable to start"**: make sure you extracted the whole zip and are opening `LearnQuick.exe` from the `bin` folder, not from inside the zip.

### Update

Close LearnQuick, delete the old folder and extract the new zip in its place. Your progress is kept elsewhere (see [your data](#your-data)), so it carries over.

### Uninstall

Delete the folder you extracted. To also delete your progress, delete `%APPDATA%\LearnQuick` (paste it into the File Explorer address bar).

## macOS

### Install

1. Double-click `LearnQuick-macos.zip`. It gives you **LearnQuick.dmg**.
2. Open `LearnQuick.dmg` and drag **LearnQuick** onto **Applications**.
3. Eject the disk image (the ⏏ button next to it in Finder) and open LearnQuick from Applications or Launchpad.

### The first time

LearnQuick is not signed with an Apple developer certificate yet, so macOS stops it the first time with *Apple could not verify "LearnQuick"*.

- **macOS 15 Sequoia or later:** click **Done**. Open **System Settings → Privacy & Security**, scroll down to the message about LearnQuick, click **Open Anyway** and confirm with your password or Touch ID.
- **macOS 14 Sonoma:** in Applications, right-click (or Control-click) LearnQuick → **Open**, then **Open** again.

After that it opens normally. Each new version asks once.

If macOS says the app **is damaged and can't be opened**, the download was marked as coming from the internet. Open **Terminal** and run:

```sh
xattr -dr com.apple.quarantine /Applications/LearnQuick.app
```

Then open LearnQuick again.

### Update

Quit LearnQuick, then drag the new version onto Applications and choose **Replace**. Your progress is kept elsewhere, so it carries over.

### Uninstall

Drag LearnQuick from Applications to the Trash. To also delete your progress, in Finder choose **Go → Go to Folder…**, type `~/Library/Application Support/LearnQuick` and move that folder to the Trash.

## Your data

Your progress lives in one file on your computer, `learnquick.sqlite3`:

- **Windows:** `%APPDATA%\LearnQuick\LearnQuick\`
- **macOS:** `~/Library/Application Support/LearnQuick/LearnQuick/`

When a new version needs to change how that file is organised, it first saves a copy beside it (`learnquick.sqlite3.v<N>.bak`). If the copy cannot be saved, nothing is changed.

To move to another computer, or just to be safe, use **Settings → Save a backup…** and, on the other computer, **Settings → Restore from a backup…**. A backup is a single `.json` file, and the web app can open it too.

## If there is no sound

LearnQuick speaks with a Spanish voice that comes inside the app. If that voice cannot start, it uses one of your system's Spanish voices instead, and tells you on screen how to add one if there is none:

- **Windows:** **Settings → Time & language → Speech → Add voices**, then choose a Spanish voice.
- **macOS:** **System Settings → Accessibility → Spoken Content → System voice → Manage Voices…**, then choose a Spanish voice.

Also check that your volume is up and the right output device is selected.

## The web app

No installation needed:

1. On the repository's main page, click **Code → Download ZIP** and extract it.
2. Double-click **`LearnQuick.html`**. It opens in your browser.

Its progress is saved in that browser, on that computer. Use **Settings → Download backup** every week or so to keep a copy.

## Building it yourself

To compile the desktop app from source, see the [desktop app's notes](../desktop/README.md) (in Spanish).
