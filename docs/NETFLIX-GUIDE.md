# Count the Spanish you watch on Netflix

*Desktop app only (Windows and Mac). The same guide is inside the app: Immersion → Import what you watched → Step-by-step guide.*

Watching series in Spanish is some of the best listening practice there is. LearnQuick can add it to your immersion log, so it counts towards your listening hours and your Spanish time for the day.

## Why you download a file instead of signing in

Streaming services don't let other apps read your account. Netflix closed its public API in 2014, and HBO Max, Prime Video and Disney+ never had one. LearnQuick also never asks for your password, because a password would open your whole account to the app.

Netflix does let you download your own viewing history. You download that file and LearnQuick reads it on your computer. **It is never sent anywhere.**

## On Windows

1. **Open Netflix on this computer.** Use Edge, Chrome or Firefox: the Netflix app, the phone and the TV have no download. In LearnQuick, press **Open Netflix**, sign in if asked and choose your profile. You land on *Viewing activity*.
2. **Download all.** Scroll to the very bottom of the list and press **Download all**. The browser saves `NetflixViewingHistory.csv` in your *Downloads* folder (`Ctrl+J` shows it).
3. **Come back to LearnQuick.** It looks in *Downloads* by itself and shows *Your Netflix history is in Downloads*. Press **Open it**.
   If nothing appears, try these:
   - press **Look in Downloads**;
   - press **Choose a file…**;
   - drag the file from File Explorer onto the card.
4. **Tick and import.** Tick the shows you watched **with Spanish audio**. The button *Tick the … made in Spanish* helps here. Then set the minutes per episode and press **Import**.

## On a Mac

1. **Open Netflix on this Mac.** Use Safari, Chrome or Firefox: the Netflix app on iPhone, iPad or the TV has no download. In LearnQuick, press **Open Netflix**, sign in if asked and choose your profile.
2. **Download all.** Scroll to the very bottom and press **Download all**. Safari saves `NetflixViewingHistory.csv` in *Downloads*; the arrow at the top right of Safari shows it.
3. **Come back and press Look in Downloads.** The first time, macOS asks whether *LearnQuick would like to access files in your Downloads folder*. Press **Allow**. The app only looks for the Netflix file and reads it on your Mac. Then press **Open it**.
4. **Tick and import.** This step is the same as on Windows.

**Pressed *Don't Allow*?** You can turn the permission on, or skip it altogether:

- **Turn it on:** open **System Settings → Privacy & Security → Files & Folders → LearnQuick**, then turn on **Downloads Folder**.
- **Skip it:** press **Choose a file…**, or drag the file from Finder onto the card. Neither needs the permission.

**macOS asks again after an update?** That happens while the app is not signed by a registered developer. Press **Allow** again.

## Real minutes and every profile

The *Download all* file lists titles and dates, but not how long you watched. That's why LearnQuick asks how many minutes an episode lasts (40 by default).

For real minutes and every profile on the account, request the full download:

1. Ask for it at **netflix.com/account/getmyinfo**. Netflix emails you when it is ready, which can take up to 30 days.
2. Download the zip and unzip it:
   - **Windows:** right-click the zip in *Downloads* → **Extract All… → Extract**;
   - **Mac:** Safari unzips it into a `netflix-report` folder in *Downloads* by itself. With Chrome or Firefox, double-click the zip in Finder.
3. Back in LearnQuick, the app finds `netflix-report/CONTENT_INTERACTION/ViewingActivity.csv`. When the file has more than one profile, choose yours.

## What counts, and what doesn't

- **You decide what was in Spanish.** No Netflix file says which audio language was on, so nothing is ticked for you.
  - Shows made in Spanish (*La casa de papel* / *Money Heist*, *Élite*, *Club de Cuervos*…) are marked, and one button ticks them all.
  - Netflix plays the dub in your profile's language unless you change it, so check before you tick.
- **Each episode counts once,** on the day you watched it. Import the same file again and only new episodes are added. Shows you imported before come back ticked.
- **It counts as listening, not as study.** It adds to your listening hours and to that day's Spanish time, but it earns **no XP**. Your streak never changes after the fact.
- **It starts when your plan started.** Only days from the start of your plan count, unless you tick *Also count what you watched before your plan began*. Days after today never count.
- **Trailers, previews and anything under 2 minutes** are left out.
- **Imported by mistake?** Delete it under *Recent activity*, like any other entry.

## Other services

HBO Max, Prime Video and Disney+ have no history download. If you keep track yourself, save a spreadsheet as CSV with a `Title` column and a `Date` column, and a `Minutes` column if you know them. It opens with **Choose a file…**.

## Something isn't right

| Problem | Fix |
|---|---|
| There is no *Download all* button | It only exists on the website in a computer's browser, at the bottom of *Viewing activity*. |
| *Your Netflix history is in Downloads* never appears | The app only offers files from the last two weeks that still have viewing you haven't imported. Press **Choose a file…** to open any other. |
| The dates look a month off | The file writes dates as your Netflix country does (3/4/26). When no date in the file settles the order, LearnQuick follows how your computer writes dates. |
| "This is not a viewing history LearnQuick can read" | You opened another file. The right one is `NetflixViewingHistory.csv` or `ViewingActivity.csv`. |
