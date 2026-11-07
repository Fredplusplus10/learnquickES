import QtQuick
import QtQuick.Controls.Basic

// Plays Spanish with the app's voice, or the system's. Hidden when there is no Spanish voice at all.
AbstractButton {
    id: control

    property string spanish
    property bool slow: false
    property string voice  // a dialogue character's voice (lq::voiceFor); empty for the narrator
    property int diameter: 38

    visible: Speech.available
    implicitWidth: diameter
    implicitHeight: diameter
    focusPolicy: Qt.NoFocus  // keeps the keyboard in the answer field
    onClicked: Speech.say(spanish, slow, voice)

    ToolTip.visible: hovered
    ToolTip.delay: 600
    ToolTip.text: slow ? "Listen slowly" : "Listen"

    contentItem: Item {
        Icon {
            anchors.centerIn: parent
            name: control.slow ? "turtle" : "speaker"
            size: control.diameter * 0.5
            color: Theme.accentStrong
        }
    }

    background: Rectangle {
        radius: height / 2
        color: control.down ? Theme.accentSoft : control.hovered ? Theme.hover : Theme.panel
        border.color: control.hovered ? Theme.accent : Theme.line
    }
}
