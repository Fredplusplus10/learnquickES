import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// One option in a set (level, plan length). The owner binds `checked` and handles the click.
AbstractButton {
    id: control

    property string heading
    property string detail
    property string badge

    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    padding: 16
    focusPolicy: Qt.StrongFocus

    contentItem: ColumnLayout {
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Text {
                text: control.heading
                color: Theme.ink
                font.pixelSize: 16
                font.weight: Font.Bold
                Layout.fillWidth: true
                // As tall as the badge, so headings line up whether or not a card has one.
                Layout.minimumHeight: 22
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap
            }
            Rectangle {
                visible: control.badge !== ""
                implicitWidth: badgeText.implicitWidth + 14
                implicitHeight: 22
                radius: 11
                color: Theme.success

                Text {
                    id: badgeText
                    anchors.centerIn: parent
                    text: control.badge
                    color: Theme.inkOnAccent
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
            }
        }
        Text {
            text: control.detail
            color: Theme.muted
            font.pixelSize: 13
            lineHeight: 1.2
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }
    }

    background: Rectangle {
        radius: Theme.radius
        color: control.checked ? Theme.accentSoft : control.hovered ? Theme.background : Theme.panel
        border.color: control.checked || control.visualFocus ? Theme.accent : Theme.line
        border.width: control.checked || control.visualFocus ? 2 : 1
    }
}
