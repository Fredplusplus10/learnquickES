import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A full on-screen keyboard: QWERTY letters, a row with the Spanish letters and marks an English keyboard
// lacks, and numbers and symbols. It types into `target` without taking its focus, so the physical keyboard
// keeps working alongside it.
Item {
    id: keyboard

    property var target: null       // a TextField or a TextArea
    property bool multiline: false  // Enter starts a new line instead of submitting
    // What Enter does when it submits ("Check"): shown on the key, which then looks like the main button.
    property string enterText: ""
    property bool enterEnabled: target !== null
    signal submitted

    // 0 off, 1 for the next character, 2 locked until pressed again.
    property int shift: 0
    property bool symbols: false
    readonly property bool typing: target !== null && !target.readOnly

    readonly property real gap: 6
    // Every row is twelve key widths; the wide keys (delete, Enter, shift, space) take what the others leave.
    readonly property real unit: Math.max(26, Math.min(44, (width - 11 * gap) / 12))
    readonly property real keyHeight: Math.round(unit * 0.84)

    readonly property var spanish: ["á", "é", "í", "ó", "ú", "ü", "ñ", "¿", "¡"]
    readonly property var rows: symbols ? [
        ["1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "delete"],
        ["@", "#", "€", "%", "&", "(", ")", "\"", ":", "enter"],
        ["-", "_", "/", "+", "=", "*", ";", "«", "»", ",", ".", "?"],
        ["letters", "space", "!"]
    ] : [
        ["q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "delete"],
        ["a", "s", "d", "f", "g", "h", "j", "k", "l", "enter"],
        ["shift", "z", "x", "c", "v", "b", "n", "m", ",", ".", "?"],
        ["symbols", "space", "!"]
    ]

    implicitWidth: 12 * 44 + 11 * gap
    implicitHeight: column.implicitHeight

    function type(text) {
        if (!typing)
            return
        if (target.selectedText.length > 0)
            target.remove(target.selectionStart, target.selectionEnd)
        target.insert(target.cursorPosition, text)
        target.forceActiveFocus()
        if (shift === 1)
            shift = 0
    }

    function erase() {
        if (!typing)
            return
        if (target.selectedText.length > 0)
            target.remove(target.selectionStart, target.selectionEnd)
        else if (target.cursorPosition > 0)
            target.remove(target.cursorPosition - 1, target.cursorPosition)
        target.forceActiveFocus()
    }

    function press(key) {
        switch (key) {
        case "shift":
            shift = (shift + 1) % 3
            break
        case "symbols":
        case "letters":
            symbols = key === "symbols"
            break
        case "delete":
            erase()
            break
        case "space":
            type(" ")
            break
        case "enter":
            if (multiline)
                type("\n")
            else
                submitted()
            break
        default:
            type(shift > 0 ? key.toUpperCase() : key)
        }
    }

    component Key: AbstractButton {
        id: key

        required property string modelData
        property bool spanish: false
        readonly property bool wide: ["delete", "enter", "shift", "space"].indexOf(modelData) >= 0
        readonly property bool toggle: modelData === "symbols" || modelData === "letters"
        readonly property string glyph: modelData.length === 1 && keyboard.shift > 0 ? modelData.toUpperCase()
                                                                                    : modelData
        readonly property bool locked: modelData === "shift" && keyboard.shift === 2
        readonly property bool submits: modelData === "enter" && keyboard.enterText !== "" && !keyboard.multiline

        text: submits ? keyboard.enterText
                     : ({ delete: "Delete", enter: "Enter", shift: "Shift", space: "Space", symbols: "Numbers and symbols",
                          letters: "Letters" })[modelData] ?? glyph
        focusPolicy: Qt.NoFocus
        autoRepeat: modelData === "delete"
        // After the check the answer is locked, but Enter still moves on.
        enabled: modelData === "enter" && !keyboard.multiline ? keyboard.enterEnabled : keyboard.typing
        implicitHeight: keyboard.keyHeight
        Layout.fillWidth: wide || spanish
        Layout.preferredWidth: toggle ? 2 * keyboard.unit + keyboard.gap : keyboard.unit
        onClicked: keyboard.press(modelData)

        contentItem: Item {
            Row {
                anchors.centerIn: parent
                spacing: 6

                Icon {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: name !== ""
                    name: ({ delete: "backspace", enter: "enter", shift: "shift" })[key.modelData] ?? ""
                    size: 18
                    color: key.locked || (key.submits && key.enabled) ? Theme.inkOnAccent : key.enabled ? Theme.ink : Theme.faint
                }
                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: text !== ""
                    text: key.submits ? key.text
                        : key.modelData === "symbols" ? "123" : key.modelData === "letters" ? "ABC"
                        : key.modelData === "space" ? "space" : key.wide ? "" : key.glyph
                    color: key.submits ? (key.enabled ? Theme.inkOnAccent : Theme.faint)
                         : !key.enabled ? Theme.faint : key.spanish ? Theme.accentStrong
                         : key.wide || key.toggle ? Theme.muted : Theme.ink
                    font.pixelSize: key.submits ? 15 : key.wide || key.toggle ? 13 : 17
                    font.weight: key.spanish || key.submits ? Font.DemiBold : Font.Normal
                }
            }
        }
        background: Rectangle {
            radius: 8
            color: key.locked ? Theme.accent
                 : key.submits && key.enabled ? (key.down || key.hovered ? Theme.accentPressed : Theme.accent)
                 : key.down || (key.modelData === "shift" && keyboard.shift === 1) ? Theme.accentSoft
                 : key.spanish ? (key.hovered ? Theme.accentHover : Theme.accentSoft)
                 : key.wide || key.toggle ? (key.hovered ? Theme.line : Theme.sidebar)
                 : key.hovered ? Theme.sidebar : Theme.panel
            border.color: key.submits && key.enabled ? "transparent" : key.spanish ? Theme.accentLine : Theme.line
        }
    }

    ColumnLayout {
        id: column
        width: parent.width
        spacing: keyboard.gap

        // What English keyboards lack, on a row of its own so it is the first thing seen.
        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 4
            spacing: keyboard.gap

            Repeater {
                model: keyboard.spanish

                delegate: Key {
                    spanish: true
                }
            }
        }
        Repeater {
            model: keyboard.rows

            delegate: RowLayout {
                id: row
                required property var modelData
                Layout.fillWidth: true
                spacing: keyboard.gap

                Repeater {
                    model: row.modelData

                    delegate: Key {}
                }
            }
        }
    }
}
