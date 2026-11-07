import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Picks the streak's icon, as Snapchat lets you: the app's flame, a suggested emoji, or any emoji typed or
// pasted from the system's emoji panel. The choice is saved in the learner's profile straight away.
Popup {
    id: picker

    required property ProfileController profile

    // Flags are left out: Windows has no flag emoji and shows two letters instead.
    readonly property var suggestions: ["", "🔥", "⚡", "⭐", "🌟", "💪", "🚀", "🎯", "🏆", "💯", "❤️", "💜", "🌮",
                                        "🌶️", "🌵", "☀️", "🌈", "🦅", "🐉", "🐢", "📚", "🧠", "😎", "🎸", "☕"]
    property bool invalid: false

    function choose(text) {
        if (picker.profile.setStreakEmoji(text)) {
            picker.close()
        } else {
            picker.invalid = text.trim() !== ""
        }
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    width: Math.min(460, parent ? parent.width - 32 : 460)
    modal: true
    focus: true
    padding: 24
    onOpened: {
        invalid = false
        field.text = ""
    }

    Overlay.modal: Rectangle {
        color: Theme.scrim
    }
    background: Rectangle {
        radius: Theme.radius
        color: Theme.panel
        border.color: Theme.line
    }

    contentItem: ColumnLayout {
        spacing: 14

        Text {
            text: "Your streak icon"
            color: Theme.ink
            font.pixelSize: 20
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            text: "Pick any emoji to stand for your streak, or keep the flame."
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }

        GridLayout {
            Layout.alignment: Qt.AlignHCenter
            columns: 5
            rowSpacing: 8
            columnSpacing: 8

            Repeater {
                model: picker.suggestions

                delegate: AbstractButton {
                    id: choice
                    required property string modelData
                    readonly property bool current: picker.profile.streakEmoji === modelData
                    implicitWidth: 58
                    implicitHeight: 58
                    focusPolicy: Qt.NoFocus
                    onClicked: picker.choose(modelData)
                    ToolTip.visible: hovered && modelData === ""
                    ToolTip.text: "LearnQuick's flame"
                    ToolTip.delay: 400
                    contentItem: Item {
                        StreakIcon {
                            anchors.centerIn: parent
                            emoji: choice.modelData
                            size: 32
                        }
                    }
                    background: Rectangle {
                        radius: Theme.smallRadius
                        color: choice.current ? Theme.accentSoft : choice.hovered ? Theme.sidebar : Theme.raised
                        border.color: choice.current ? Theme.accent : Theme.line
                        border.width: choice.current ? 2 : 1
                    }
                }
            }
        }

        Text {
            text: "Or any other emoji"
            color: Theme.ink
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            TextField {
                id: field
                Layout.fillWidth: true
                placeholderText: "Type or paste an emoji"
                font.pixelSize: 18
                color: Theme.ink
                leftPadding: 12
                rightPadding: 12
                onTextEdited: picker.invalid = false
                onAccepted: picker.choose(text)
                background: Rectangle {
                    implicitHeight: 44
                    radius: Theme.smallRadius
                    color: Theme.panel
                    border.color: picker.invalid ? Theme.danger : field.activeFocus ? Theme.accent : Theme.line
                    border.width: field.activeFocus || picker.invalid ? 2 : 1
                }
            }
            PrimaryButton {
                text: "Use it"
                enabled: field.text.trim() !== ""
                onClicked: picker.choose(field.text)
            }
        }
        Text {
            Layout.fillWidth: true
            text: picker.invalid ? "That is not an emoji. Try another one."
                                 : Qt.platform.os === "osx" ? "Control + Command + Space opens the Mac's emoji panel."
                                 : "Windows key + . (period) opens the Windows emoji panel."
            color: picker.invalid ? Theme.danger : Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        PrimaryButton {
            Layout.alignment: Qt.AlignRight
            secondary: true
            text: "Close"
            onClicked: picker.close()
        }
    }
}
