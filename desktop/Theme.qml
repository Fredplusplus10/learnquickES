pragma Singleton
import QtQuick

// Design tokens from the LearnQuick design kit (app/css/kit.css), shared by every screen, in a day and a night
// version. Settings picks one, or follows the computer ("system").
QtObject {
    readonly property bool dark: Preferences.theme === "dark"
                                 || (Preferences.theme === "system" && Qt.styleHints.colorScheme === Qt.ColorScheme.Dark)

    readonly property color background: dark ? "#1b1512" : "#faf7f4"
    readonly property color panel: dark ? "#241d19" : "#fffdfb"
    readonly property color sidebar: dark ? "#201916" : "#f6f0ed"
    readonly property color sidebarHover: dark ? "#2d2420" : "#efe6e1"
    readonly property color raised: dark ? "#2b231f" : "#ffffff"  // answer tiles, word cards, text areas
    readonly property color hover: dark ? "#332823" : "#fff6ef"
    readonly property color ink: dark ? "#f4ebe6" : "#36231e"
    readonly property color muted: dark ? "#b8aaa2" : "#796f6a"
    readonly property color faint: dark ? "#7d6f68" : "#b9aea8"
    readonly property color inkOnAccent: "#ffffff"  // text and icons on accent, success and toast fills
    readonly property color accent: dark ? "#f07a3a" : "#ed712f"
    readonly property color accentStrong: dark ? "#ff9459" : "#d8541b"
    readonly property color accentPressed: dark ? "#d45d24" : "#d8541b"  // a filled button under the pointer
    readonly property color accentSoft: dark ? "#3a2416" : "#fff0e5"
    readonly property color accentHover: dark ? "#4a2d1b" : "#ffe4d1"
    readonly property color accentLine: dark ? "#6b3d22" : "#f5cdb2"
    readonly property color hoverLine: dark ? "#8a5230" : "#f2b68e"
    readonly property color line: dark ? "#3a2f29" : "#e9ded7"
    readonly property color edge: dark ? "#4a3d36" : "#e2d5cc"  // borders of things to click
    readonly property color success: dark ? "#57b076" : "#3d8b59"
    readonly property color successSoft: dark ? "#1c3124" : "#e9f5e9"
    readonly property color blue: dark ? "#6aa9e6" : "#4287c8"
    readonly property color blueSoft: dark ? "#1b2a3b" : "#eaf2fa"
    readonly property color purple: dark ? "#a990e6" : "#8067bc"
    readonly property color purpleSoft: dark ? "#2a2340" : "#f1edf9"
    readonly property color danger: dark ? "#e8735f" : "#b8402f"
    readonly property color dangerSoft: dark ? "#3b1f1a" : "#fdeeed"
    readonly property color goodInk: dark ? "#8fd8a6" : "#24603a"
    readonly property color goodLine: dark ? "#2e5a3c" : "#c9e5cf"
    readonly property color badInk: dark ? "#f4a597" : "#8c2f22"
    readonly property color badLine: dark ? "#6b3128" : "#f2cbc6"
    readonly property color toast: dark ? "#3b302a" : "#36231e"
    readonly property color toastMuted: dark ? "#cbbdb5" : "#e7d9d2"
    readonly property color toastIcon: "#ffcf9f"
    readonly property color scrim: dark ? "#99000000" : "#5536231e"  // behind a dialog
    // Article colours: learn every noun with its gender.
    readonly property color masculine: dark ? "#74aee8" : "#2f6fae"
    readonly property color feminine: dark ? "#ea7fa0" : "#b23a5e"

    readonly property int radius: 16
    readonly property int smallRadius: 10

    function tone(name) {
        switch (name) {
        case "blue": return blue
        case "purple": return purple
        case "green": return success
        default: return accent
        }
    }

    // Calendar and week-strip days, by the state the engine reports.
    function dayFill(state) {
        switch (state) {
        case "done": return accent
        case "frozen": return blueSoft
        case "today": return accentSoft
        case "missed": return line
        default: return "transparent"
        }
    }

    function dayInk(state) {
        switch (state) {
        case "done": return inkOnAccent
        case "frozen": return blue
        case "today": return accentStrong
        case "missed": return muted
        case "future": return ink
        default: return faint
        }
    }
}
