import QtQuick
import QtQuick.Controls.Basic

// A one-line text box in the app's style.
TextField {
    id: field

    placeholderTextColor: Theme.faint
    color: Theme.ink
    font.pixelSize: 15
    padding: 11
    selectByMouse: true

    background: Rectangle {
        radius: Theme.smallRadius
        color: Theme.raised
        border.color: field.activeFocus ? Theme.accent : Theme.edge
        border.width: field.activeFocus ? 2 : 1
    }
}
