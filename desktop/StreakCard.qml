import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The streak with the learner's own icon, this week day by day and the streak freezes. Today and Progress.
Card {
    id: card

    required property TodayModel today
    required property ProfileController profile
    property bool canChangeIcon: true
    signal changeIcon()

    RowLayout {
        Layout.fillWidth: true
        spacing: 12

        StreakIcon {
            emoji: card.profile.streakEmoji
            lit: card.today.streak > 0
            size: 34
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 1

            Text {
                Layout.fillWidth: true
                text: card.today.streak + "-day streak"
                color: Theme.ink
                font.pixelSize: 20
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: card.today.todayDone ? "Today is done" : "Earn 10 XP today to keep it going."
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
        }
        Text {
            Layout.alignment: Qt.AlignTop
            text: "Longest: " + card.today.longestStreak
            color: Theme.muted
            font.pixelSize: 13
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 4

        Repeater {
            model: card.today.week

            delegate: ColumnLayout {
                id: weekDay
                required property var modelData
                readonly property bool done: modelData.state === "done"
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 4

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    text: weekDay.modelData.label
                    color: Theme.muted
                    font.pixelSize: 11
                }
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    implicitWidth: 30
                    implicitHeight: 30
                    radius: 15
                    color: Theme.dayFill(weekDay.modelData.state)
                    border.color: weekDay.modelData.state === "today" ? Theme.accent
                                : weekDay.done ? Theme.accent : Theme.line
                    border.width: 1.5

                    Icon {
                        anchors.centerIn: parent
                        visible: weekDay.done
                        name: "check"
                        color: Theme.inkOnAccent
                        size: 16
                    }
                    Text {
                        anchors.centerIn: parent
                        visible: !weekDay.done
                        text: weekDay.modelData.day
                        color: Theme.dayInk(weekDay.modelData.state)
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                    }
                }
            }
        }
    }
    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        Repeater {
            model: card.today.maxFreezes

            delegate: Rectangle {
                required property int index
                implicitWidth: 12
                implicitHeight: 12
                radius: 6
                color: index < card.today.freezes ? Theme.blue : Theme.line
            }
        }
        Text {
            Layout.leftMargin: 4
            Layout.fillWidth: true
            text: card.today.freezes + "/" + card.today.maxFreezes + " streak freezes · days off never break it"
            color: Theme.muted
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
        AbstractButton {
            id: changeIcon
            visible: card.canChangeIcon
            focusPolicy: Qt.StrongFocus
            onClicked: card.changeIcon()
            contentItem: Text {
                text: "Change icon"
                color: changeIcon.hovered || changeIcon.visualFocus ? Theme.accentStrong : Theme.accent
                font.pixelSize: 13
                font.weight: Font.DemiBold
                font.underline: changeIcon.hovered || changeIcon.visualFocus
            }
            background: null
        }
    }
}
