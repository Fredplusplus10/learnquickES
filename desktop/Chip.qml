import QtQuick
import QtQuick.Controls.Basic

// A pill that shows whether it is selected; the owner decides what a click changes.
AbstractButton {
    id: control

    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: 38
    leftPadding: 16
    rightPadding: 16
    focusPolicy: Qt.StrongFocus

    contentItem: Text {
        text: control.text
        color: control.checked ? Theme.accentStrong : Theme.ink
        font.pixelSize: 14
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: height / 2
        color: control.checked ? Theme.accentSoft : control.hovered ? Theme.sidebar : Theme.panel
        border.color: control.checked || control.visualFocus ? Theme.accent : Theme.line
        border.width: control.visualFocus ? 2 : 1
    }
}
