import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The coach's suggestions: what to do and why, from the learner's own numbers. Each one can open its screen.
Card {
    id: card

    property var tips: []
    property int limit: 3
    property string more  // a link to the coach screen when set
    signal follow(string route)

    readonly property var shown: tips.slice(0, limit)

    title: "Your coach"
    subtitle: tips.length > 0 ? "Suggestions, not orders" : ""

    Text {
        Layout.fillWidth: true
        visible: card.tips.length === 0
        text: "Nothing to suggest right now. ¡Sigue así!"
        color: Theme.muted
        font.pixelSize: 14
        wrapMode: Text.WordWrap
    }

    Repeater {
        model: card.shown

        delegate: ColumnLayout {
            id: tip
            required property var modelData
            required property int index
            Layout.fillWidth: true
            spacing: 3

            Rectangle {
                visible: tip.index > 0
                Layout.fillWidth: true
                Layout.bottomMargin: 8
                implicitHeight: 1
                color: Theme.line
            }
            Text {
                Layout.fillWidth: true
                text: tip.modelData.title
                color: Theme.ink
                font.pixelSize: tip.index === 0 ? 16 : 14
                font.weight: Font.DemiBold
                wrapMode: Text.WordWrap
                textFormat: Text.PlainText
            }
            Text {
                Layout.fillWidth: true
                text: tip.modelData.detail
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
                textFormat: Text.PlainText
            }
            AbstractButton {
                id: go
                visible: tip.modelData.route !== ""
                Layout.topMargin: 2
                focusPolicy: Qt.StrongFocus
                onClicked: card.follow(tip.modelData.route)

                contentItem: RowLayout {
                    spacing: 2

                    Text {
                        text: tip.index === 0 ? "Start" : "Open"
                        color: go.hovered || go.visualFocus ? Theme.accentStrong : Theme.accent
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        font.underline: go.visualFocus
                    }
                    Icon {
                        name: "chevron-right"
                        size: 14
                        color: go.hovered || go.visualFocus ? Theme.accentStrong : Theme.accent
                    }
                }
            }
        }
    }

    AbstractButton {
        id: moreLink
        visible: card.more !== ""
        Layout.topMargin: 4
        focusPolicy: Qt.StrongFocus
        onClicked: card.follow("coach")

        contentItem: Text {
            text: card.more
            color: moreLink.hovered || moreLink.visualFocus ? Theme.accentStrong : Theme.muted
            font.pixelSize: 13
            font.underline: true
        }
    }
}
