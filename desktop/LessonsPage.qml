import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The course map: 25 lessons in five phases, each phase closed by its checkpoint (the last one, the B1 mock).
ScrollPage {
    id: page

    required property LearnerController learner
    maxWidth: 1240

    readonly property var phases: learner.courseMap
    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    // Lessons per row: three on a wide page, two on a medium one, one below that.
    readonly property int lessonColumns: columnWidth >= 1100 ? 3 : columnWidth >= 760 ? 2 : 1
    readonly property int doneCount: phases.reduce((sum, phase) => sum + phase.done, 0)
    readonly property int lessonCount: phases.reduce((sum, phase) => sum + phase.units.length, 0)
    // Today's lesson in the plan, or else the first one not done yet.
    readonly property var nextUnit: {
        let first = null
        for (const phase of phases) {
            for (const unit of phase.units) {
                if (unit.state === "today")
                    return unit
                if (first === null && unit.state !== "done")
                    first = unit
            }
        }
        return first
    }

    PageHero {
        overline: "The course"
        title: "Lessons"
        subtitle: "25 lessons in five phases, from your first conversation to B1. Follow the plan, which suggests one "
                  + "a day, or open any lesson to review it."
    }

    // How far through the course, and the way back in.
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: summaryRow.implicitHeight + 30
        radius: Theme.radius
        color: Theme.accentSoft
        border.color: Theme.accentLine

        RowLayout {
            id: summaryRow
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 20 }
            spacing: 14

            Icon {
                name: "book"
                color: Theme.accent
                size: 26
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: page.doneCount + " of " + page.lessonCount + " lessons completed"
                    color: Theme.ink
                    font.pixelSize: 17
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "Five phases · a checkpoint at the end of each, and the B1 mock exam to close the course"
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
            PrimaryButton {
                visible: page.nextUnit !== null
                text: page.nextUnit === null ? ""
                      : (page.nextUnit.state === "today" ? "Today's lesson: " : "Continue: lesson ") + page.nextUnit.n
                onClicked: page.openRoute("lesson/" + page.nextUnit.index)
            }
        }
    }

    Repeater {
        model: page.phases

        delegate: Card {
            id: phaseCard
            required property var modelData
            // The phase of the lesson the learner continues with.
            readonly property bool current: page.nextUnit !== null && modelData.units.some(unit => unit.index === page.nextUnit.index)
            readonly property bool complete: modelData.done === modelData.units.length
            Layout.fillWidth: true
            Accessible.role: Accessible.Grouping
            Accessible.name: "Phase " + modelData.n + ", " + modelData.title + ": " + modelData.done + " of "
                             + modelData.units.length + " lessons complete"

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 4

                    Text {
                        Layout.fillWidth: true
                        text: "Phase " + phaseCard.modelData.n + " · " + phaseCard.modelData.title
                        textFormat: Text.PlainText
                        color: Theme.ink
                        font.pixelSize: 21
                        font.weight: Font.Bold
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth: true
                        text: phaseCard.modelData.description
                        textFormat: Text.PlainText
                        color: Theme.muted
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }
                }
                Rectangle {
                    Layout.alignment: Qt.AlignTop
                    implicitWidth: countRow.implicitWidth + 24
                    implicitHeight: 30
                    radius: 15
                    color: phaseCard.complete ? Theme.successSoft : phaseCard.current ? Theme.accentSoft : Theme.background
                    border.color: phaseCard.complete ? Theme.goodLine : phaseCard.current ? Theme.accentLine : Theme.line

                    RowLayout {
                        id: countRow
                        anchors.centerIn: parent
                        spacing: 5

                        Icon {
                            visible: phaseCard.complete
                            name: "check"
                            size: 14
                            color: Theme.goodInk
                        }
                        Text {
                            text: phaseCard.modelData.done + " of " + phaseCard.modelData.units.length + " complete"
                            color: phaseCard.complete ? Theme.goodInk : phaseCard.current ? Theme.accentStrong : Theme.muted
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                        }
                    }
                }
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                columns: page.lessonColumns
                rowSpacing: 10
                columnSpacing: 10

                Repeater {
                    model: phaseCard.modelData.units

                    delegate: AbstractButton {
                        id: unitTile
                        required property var modelData
                        readonly property bool done: modelData.state === "done"
                        readonly property bool isToday: modelData.state === "today"
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true  // a row's tiles share the tallest one's height
                        implicitHeight: Math.max(96, tileRow.implicitHeight + 28)
                        focusPolicy: Qt.StrongFocus
                        onClicked: page.openRoute("lesson/" + modelData.index)
                        Keys.onReturnPressed: clicked()
                        Keys.onEnterPressed: clicked()
                        padding: 14
                        Accessible.name: "Lesson " + modelData.n + ": " + modelData.title
                                         + (done ? ", completed, best " + modelData.best + "%" : isToday ? ", today's lesson" : "")
                        // The full title when two lines are not enough for it.
                        ToolTip.visible: (hovered || visualFocus) && titleText.truncated
                        ToolTip.text: modelData.title
                        ToolTip.delay: 400

                        background: Rectangle {
                            radius: Theme.smallRadius
                            color: unitTile.hovered ? Theme.hover : unitTile.isToday ? Theme.accentSoft : Theme.raised
                            border.color: unitTile.visualFocus || unitTile.isToday ? Theme.accent
                                        : unitTile.hovered ? Theme.hoverLine : Theme.line
                            border.width: unitTile.visualFocus || unitTile.isToday ? 2 : 1
                        }
                        contentItem: RowLayout {
                            id: tileRow
                            spacing: 12

                            Rectangle {
                                Layout.alignment: Qt.AlignTop
                                implicitWidth: 36
                                implicitHeight: 36
                                radius: 18
                                color: unitTile.done ? Theme.success : unitTile.isToday ? Theme.accent : Theme.sidebar

                                Text {
                                    anchors.centerIn: parent
                                    visible: !unitTile.done
                                    text: unitTile.modelData.n
                                    color: unitTile.isToday ? Theme.inkOnAccent : Theme.muted
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                }
                                Icon {
                                    anchors.centerIn: parent
                                    visible: unitTile.done
                                    name: "check"
                                    color: Theme.inkOnAccent
                                    size: 18
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                spacing: 2

                                Text {
                                    id: titleText
                                    Layout.fillWidth: true
                                    text: unitTile.modelData.title
                                    textFormat: Text.PlainText
                                    color: Theme.ink
                                    font.pixelSize: 15
                                    font.weight: Font.DemiBold
                                    wrapMode: Text.WordWrap
                                    maximumLineCount: 2
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: unitTile.modelData.titleEs
                                    textFormat: Text.PlainText
                                    color: Theme.muted
                                    font.pixelSize: 13
                                    font.italic: true
                                    elide: Text.ElideRight
                                }
                                Item { Layout.fillHeight: true }
                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Text {
                                        Layout.fillWidth: true
                                        text: unitTile.done ? "Best " + unitTile.modelData.best + "%"
                                              : unitTile.isToday ? "Today's lesson" : ""
                                        color: unitTile.done ? Theme.goodInk : Theme.accentStrong
                                        font.pixelSize: 12
                                        font.weight: Font.DemiBold
                                    }
                                    Text {
                                        text: unitTile.done ? "Review" : unitTile.isToday ? "Start" : "View"
                                        color: unitTile.hovered || unitTile.visualFocus ? Theme.accentStrong : Theme.accent
                                        font.pixelSize: 12
                                        font.weight: Font.DemiBold
                                    }
                                    Icon {
                                        name: "chevron-right"
                                        size: 12
                                        color: unitTile.hovered || unitTile.visualFocus ? Theme.accentStrong : Theme.accent
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // The checkpoint that closes the phase.
            AbstractButton {
                id: checkpointRow
                readonly property bool isFinal: phaseCard.modelData.n === 5
                readonly property int best: phaseCard.modelData.checkpointBest
                Layout.fillWidth: true
                implicitHeight: 58
                focusPolicy: Qt.StrongFocus
                onClicked: page.openRoute(isFinal ? "mock" : "checkpoint/" + phaseCard.modelData.n)
                Keys.onReturnPressed: clicked()
                Keys.onEnterPressed: clicked()
                leftPadding: 16
                rightPadding: 16
                Accessible.name: (isFinal ? "B1 mock exam" : "Phase " + phaseCard.modelData.n + " checkpoint")
                                 + (best >= 0 ? ", best " + best + "%" : ", not taken yet")

                background: Rectangle {
                    radius: Theme.smallRadius
                    color: checkpointRow.hovered ? Theme.hover : Theme.background
                    border.color: checkpointRow.visualFocus ? Theme.accent : checkpointRow.hovered ? Theme.hoverLine : Theme.line
                    border.width: checkpointRow.visualFocus ? 2 : 1
                }
                contentItem: RowLayout {
                    spacing: 12

                    Rectangle {
                        implicitWidth: 34
                        implicitHeight: 34
                        radius: 17
                        color: checkpointRow.best >= 70 ? Theme.successSoft : Theme.accentSoft

                        Icon {
                            anchors.centerIn: parent
                            name: checkpointRow.isFinal ? "award" : "target"
                            size: 18
                            color: checkpointRow.best >= 70 ? Theme.goodInk : Theme.accentStrong
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Text {
                            Layout.fillWidth: true
                            text: checkpointRow.isFinal ? "B1 mock exam" : "Phase " + phaseCard.modelData.n + " checkpoint"
                            color: Theme.ink
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: checkpointRow.isFinal ? "Reading, listening, grammar, writing and speaking"
                                                        : "A short test on the phase's lessons"
                            color: Theme.muted
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                    Text {
                        text: checkpointRow.best >= 0 ? "Best " + checkpointRow.best + "%" : "Not taken yet"
                        color: checkpointRow.best >= 70 ? Theme.goodInk : Theme.muted
                        font.pixelSize: 13
                        font.weight: checkpointRow.best >= 0 ? Font.DemiBold : Font.Normal
                    }
                    Icon {
                        name: "chevron-right"
                        size: 16
                        color: checkpointRow.hovered || checkpointRow.visualFocus ? Theme.accentStrong : Theme.muted
                    }
                }
            }
        }
    }
}
