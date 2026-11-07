import QtQuick
import QtQuick.Layouts

// A titled panel. Children go into the column below the title.
Rectangle {
    id: card

    property string title
    property string subtitle
    property string icon  // an Icon name shown before the title; empty for none
    default property alias content: body.data

    readonly property int padding: 22
    implicitWidth: body.implicitWidth + 2 * padding
    implicitHeight: (title ? header.implicitHeight + 14 : 0) + body.implicitHeight + 2 * padding

    radius: Theme.radius
    color: Theme.panel
    border.color: Theme.line

    RowLayout {
        id: header
        visible: card.title !== ""
        anchors { left: parent.left; right: parent.right; top: parent.top; margins: card.padding }
        spacing: 10

        Icon {
            visible: card.icon !== ""
            name: card.icon
            color: Theme.accent
            size: 20
        }
        Text {
            text: card.title
            color: Theme.ink
            font.pixelSize: 15
            font.weight: Font.DemiBold
            Layout.fillWidth: true
        }
        Text {
            text: card.subtitle
            color: Theme.muted
            font.pixelSize: 13
        }
    }

    ColumnLayout {
        id: body
        spacing: 12
        anchors {
            left: parent.left
            right: parent.right
            top: card.title ? header.bottom : parent.top
            margins: card.padding
            topMargin: card.title ? 14 : card.padding
        }
    }
}
