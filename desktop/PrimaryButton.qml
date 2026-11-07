import QtQuick
import QtQuick.Controls.Basic

// The main call to action; `secondary` gives the quieter outlined version (Back).
Button {
    id: control

    property bool secondary: false

    implicitHeight: 46
    leftPadding: 26
    rightPadding: 26
    font.pixelSize: 15
    font.weight: Font.DemiBold

    contentItem: Text {
        text: control.text
        font: control.font
        color: control.secondary ? Theme.ink : Theme.inkOnAccent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        radius: height / 2
        color: control.secondary ? (control.hovered ? Theme.sidebar : "transparent")
                                 : !control.enabled ? Theme.faint
                                 : control.down || control.hovered ? Theme.accentPressed : Theme.accent
        border.color: control.visualFocus ? Theme.accentStrong : control.secondary ? Theme.line : "transparent"
        border.width: control.visualFocus ? 2 : 1
    }
}
