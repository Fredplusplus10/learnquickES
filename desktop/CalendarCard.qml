import QtQuick
import QtQuick.Layouts

// Five weeks, Monday to Sunday, ending with this one: days studied, covered by a freeze or missed.
Card {
    id: card

    required property TodayModel today
    title: "Calendar"
    subtitle: card.today.calendarTitle

    GridLayout {
        Layout.fillWidth: true
        columns: 7
        rowSpacing: 5
        columnSpacing: 5

        Repeater {
            model: ["M", "T", "W", "T", "F", "S", "S"]

            delegate: Text {
                required property string modelData
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                text: modelData
                color: Theme.muted
                font.pixelSize: 11
                horizontalAlignment: Text.AlignHCenter
            }
        }
        Repeater {
            model: card.today.calendar

            delegate: Rectangle {
                id: cell
                required property var modelData
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                implicitHeight: 28
                radius: 8
                color: Theme.dayFill(cell.modelData.state)
                border.color: cell.modelData.state === "today" ? Theme.accent : "transparent"
                border.width: 1.5

                Text {
                    anchors.centerIn: parent
                    text: cell.modelData.day
                    color: Theme.dayInk(cell.modelData.state)
                    font.pixelSize: 12
                }
            }
        }
    }
    RowLayout {
        spacing: 14

        Repeater {
            model: [
                { state: "done", label: "Studied" },
                { state: "frozen", label: "Freeze" },
                { state: "missed", label: "Missed" }
            ]

            delegate: RowLayout {
                id: legendItem
                required property var modelData
                spacing: 5

                Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    radius: 3
                    color: Theme.dayFill(legendItem.modelData.state)
                }
                Text {
                    text: legendItem.modelData.label
                    color: Theme.muted
                    font.pixelSize: 11
                }
            }
        }
    }
}
