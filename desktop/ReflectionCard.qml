import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// What the learner enjoyed and how hard it felt, asked every few sessions. Mixed practice leans towards what
// they enjoy and the coach takes it into account; the plan itself stays the same.
Card {
    id: card

    required property LearnerController learner
    property bool asked: false
    property string activity            // "lesson", a trainer id, or "coach" outside a session
    property var kinds: []              // [{id, label}] to choose from
    property bool askFeel: true
    property string question: "¿Qué te gustó? What did you enjoy in this session?"

    property var liked: []
    property string feel
    property string reply               // what the reflection changed, once saved
    property bool dismissed: false
    readonly property bool wanted: asked && !dismissed

    function toggle(kind) {
        liked = liked.indexOf(kind) >= 0 ? liked.filter(k => k !== kind) : liked.concat([kind])
    }

    function save() {
        reply = learner.reflect(activity, kinds.map(k => k.id), liked, feel, note.text)
    }

    // Asked again after a few more sessions on the same screen ("Another round"): start from a blank card.
    onAskedChanged: {
        if (!asked)
            return
        liked = []
        feel = ""
        reply = ""
        dismissed = false
        note.clear()
    }

    visible: wanted
    title: "Quick reflection"
    subtitle: askFeel && reply === "" ? "Every " + learner.reflectEvery + " sessions" : ""

    Text {
        Layout.fillWidth: true
        visible: card.reply === ""
        text: card.question
        color: Theme.ink
        font.pixelSize: 15
        wrapMode: Text.WordWrap
    }
    Flow {
        Layout.fillWidth: true
        visible: card.reply === ""
        spacing: 8

        Repeater {
            model: card.kinds

            delegate: Chip {
                required property var modelData
                text: modelData.label
                checked: card.liked.indexOf(modelData.id) >= 0
                onClicked: card.toggle(modelData.id)
            }
        }
    }
    Text {
        Layout.fillWidth: true
        Layout.topMargin: 4
        visible: card.reply === "" && card.askFeel
        text: "How did it feel?"
        color: Theme.ink
        font.pixelSize: 15
    }
    Flow {
        Layout.fillWidth: true
        visible: card.reply === "" && card.askFeel
        spacing: 8

        Repeater {
            model: [
                { id: "easy", label: "Too easy" },
                { id: "right", label: "Just right" },
                { id: "hard", label: "Too hard" }
            ]

            delegate: Chip {
                required property var modelData
                text: modelData.label
                checked: card.feel === modelData.id
                onClicked: card.feel = card.feel === modelData.id ? "" : modelData.id
            }
        }
    }
    TextField {
        id: note
        Layout.fillWidth: true
        Layout.topMargin: 4
        visible: card.reply === ""
        placeholderText: "Anything else? Optional, and in Spanish if you can: «Me gustó…»"
        placeholderTextColor: Theme.faint
        maximumLength: 280
        color: Theme.ink
        font.pixelSize: 15
        padding: 12
        onAccepted: if (saveButton.enabled) card.save()

        background: Rectangle {
            radius: Theme.smallRadius
            color: Theme.background
            border.color: note.activeFocus ? Theme.accent : Theme.line
            border.width: note.activeFocus ? 2 : 1
        }
    }
    RowLayout {
        Layout.fillWidth: true
        visible: card.reply === ""
        spacing: 12

        PrimaryButton {
            id: saveButton
            text: "Save"
            enabled: card.liked.length > 0 || card.feel !== "" || note.text.trim() !== ""
            onClicked: card.save()
        }
        PrimaryButton {
            visible: card.askFeel
            secondary: true
            text: "Not now"
            onClicked: {
                card.learner.postponeReflection()
                card.dismissed = true
            }
        }
    }

    // Once saved: what changes.
    RowLayout {
        Layout.fillWidth: true
        visible: card.reply !== ""
        spacing: 12

        Icon {
            Layout.alignment: Qt.AlignTop
            name: "sparkles"
            color: Theme.accentStrong
        }
        Text {
            Layout.fillWidth: true
            text: "¡Gracias! " + card.reply
            color: Theme.ink
            font.pixelSize: 15
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
        }
    }
}
