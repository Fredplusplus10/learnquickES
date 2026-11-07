import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Where a screen sits, above its hero: a link back to the screen it belongs to, then where inside it.
RowLayout {
    id: crumb

    property string backLabel
    property string trail  // shown after the link, e.g. "Phase 2 · Everyday Conversations"
    signal back()

    Layout.fillWidth: true
    Layout.bottomMargin: -10
    spacing: 6

    AbstractButton {
        id: backLink
        focusPolicy: Qt.StrongFocus
        onClicked: crumb.back()
        Keys.onReturnPressed: clicked()
        Keys.onEnterPressed: clicked()
        Accessible.name: "Back to " + crumb.backLabel.toLowerCase()
        contentItem: RowLayout {
            spacing: 4
            Icon {
                name: "chevron-left"
                size: 14
                color: backLink.hovered || backLink.visualFocus ? Theme.accentStrong : Theme.accent
            }
            Text {
                text: crumb.backLabel
                color: backLink.hovered || backLink.visualFocus ? Theme.accentStrong : Theme.accent
                font.pixelSize: 14
                font.weight: Font.DemiBold
                font.underline: backLink.visualFocus
            }
        }
        background: null
    }
    Text {
        Layout.fillWidth: true
        visible: crumb.trail !== ""
        text: "›  " + crumb.trail
        textFormat: Text.PlainText
        color: Theme.muted
        font.pixelSize: 14
        elide: Text.ElideRight
    }
}
