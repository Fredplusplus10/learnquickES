import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The test that closes a phase: 15 questions from its five lessons, no second tries, 70% to pass.
// Missed questions point back to the lessons they came from. On a wide page the phase's lessons and the rules
// sit beside the test.
ScrollPage {
    id: page

    required property LearnerController learner
    required property SessionModel session
    property int phase: 1

    readonly property var info: learner.checkpoint(phase)
    property var result: null
    readonly property bool running: session.active && !session.finished
    readonly property var phaseInfo: learner.courseMap[phase - 1] ?? ({})
    readonly property var units: phaseInfo.units ?? []
    readonly property int unitsDone: units.filter(unit => unit.state === "done").length

    maxWidth: 1240
    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 1000

    function start() {
        result = null
        session.startCheckpoint(phase)
        scrollToTop()
    }

    Connections {
        target: page.session
        function onCompleted() {
            page.result = page.session.result
        }
    }

    Keys.onReturnPressed: if (!running && result === null) start()
    Keys.onEnterPressed: if (!running && result === null) start()

    Breadcrumb {
        backLabel: "Lessons"
        trail: "Phase " + page.phase + " · " + (page.info.phaseTitle ?? "")
        onBack: page.openRoute("lessons")
    }

    PageHero {
        overline: "Checkpoint"
        title: page.info.title ?? ""
        subtitle: "15 questions from the five lessons of this phase. No second tries: it shows what is solid and what "
                  + "needs another look."
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Card {
                visible: !page.running && page.result === null
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 14

                    Icon {
                        name: "target"
                        size: 34
                        color: Theme.accent
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: "70% passes"
                            color: Theme.ink
                            font.pixelSize: 18
                            font.weight: Font.DemiBold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: (page.info.best ?? -1) >= 0 ? "Your best so far: " + page.info.best + "%. You can retake it any time."
                                                               : "About 10 minutes. You can retake it any time."
                            color: Theme.muted
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                        }
                    }
                    PrimaryButton {
                        text: "Start"
                        onClicked: page.start()
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                visible: page.running

                ExerciseSession {
                    Layout.fillWidth: true
                    focus: visible
                    session: page.session
                }
            }

            SessionSummary {
                Layout.fillWidth: true
                visible: page.result !== null
                result: page.result ?? ({})
                title: (page.result?.passed ?? false) ? "Checkpoint passed!" : "Not yet: 70% passes"
                message: page.result === null ? ""
                         : page.result.firstTry + " of " + page.result.answered + " right."
                           + ((page.result.passed ?? false) ? " The next phase is ready for you." : " Review the lessons below, then try again.")

                PrimaryButton {
                    text: "Back to Today"
                    onClicked: page.openRoute("today")
                }
                PrimaryButton {
                    secondary: true
                    text: "Try again"
                    onClicked: page.start()
                }
            }

            Card {
                visible: page.result !== null && (page.result.weakUnits ?? []).length > 0
                Layout.fillWidth: true
                title: "Lessons to review"

                Repeater {
                    model: page.result?.weakUnits ?? []

                    delegate: AbstractButton {
                        id: weak
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: 46
                        focusPolicy: Qt.TabFocus
                        onClicked: page.openRoute("lesson/" + modelData.index)
                        leftPadding: 14
                        rightPadding: 14

                        background: Rectangle {
                            radius: Theme.smallRadius
                            color: weak.hovered ? Theme.hover : Theme.background
                            border.color: weak.visualFocus ? Theme.accent : Theme.line
                        }
                        contentItem: RowLayout {
                            spacing: 10

                            Text {
                                Layout.fillWidth: true
                                text: "Lesson " + weak.modelData.n + ": " + weak.modelData.title
                                color: Theme.ink
                                font.pixelSize: 15
                                elide: Text.ElideRight
                            }
                            Text {
                                text: weak.modelData.misses === 1 ? "1 miss" : weak.modelData.misses + " misses"
                                color: Theme.danger
                                font.pixelSize: 13
                            }
                            Icon {
                                name: "chevron-right"
                                size: 16
                                color: Theme.muted
                            }
                        }
                    }
                }
            }
        }

        // What the checkpoint covers, and how it is marked.
        ColumnLayout {
            Layout.fillWidth: !page.wide
            Layout.preferredWidth: page.wide ? 340 : -1
            Layout.maximumWidth: page.wide ? 340 : -1
            Layout.alignment: Qt.AlignTop
            visible: page.wide
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "What it covers"
                subtitle: page.unitsDone + " of " + page.units.length + " done"
                icon: "book"

                Repeater {
                    model: page.units

                    delegate: AbstractButton {
                        id: unitRow
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: unitLine.implicitHeight + 14
                        focusPolicy: Qt.StrongFocus
                        onClicked: page.openRoute("lesson/" + modelData.index)
                        Keys.onReturnPressed: clicked()
                        Keys.onEnterPressed: clicked()
                        Accessible.name: "Lesson " + modelData.n + ", " + modelData.title
                        background: Rectangle {
                            radius: Theme.smallRadius
                            color: unitRow.hovered ? Theme.hover : "transparent"
                            border.color: unitRow.visualFocus ? Theme.accent : "transparent"
                        }
                        contentItem: Item {
                            RowLayout {
                                id: unitLine
                                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; leftMargin: 6; rightMargin: 6 }
                                spacing: 10

                                Rectangle {
                                    implicitWidth: 24
                                    implicitHeight: 24
                                    radius: 12
                                    color: unitRow.modelData.state === "done" ? Theme.success : Theme.background
                                    border.color: unitRow.modelData.state === "done" ? Theme.success : Theme.line

                                    Icon {
                                        anchors.centerIn: parent
                                        visible: unitRow.modelData.state === "done"
                                        name: "check"
                                        size: 13
                                        color: Theme.inkOnAccent
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        visible: unitRow.modelData.state !== "done"
                                        text: unitRow.modelData.n
                                        color: Theme.muted
                                        font.pixelSize: 11
                                        font.weight: Font.DemiBold
                                    }
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: unitRow.modelData.title
                                    color: Theme.ink
                                    font.pixelSize: 13
                                    elide: Text.ElideRight
                                    textFormat: Text.PlainText
                                }
                                Text {
                                    visible: unitRow.modelData.best >= 0
                                    text: unitRow.modelData.best + "%"
                                    color: Theme.goodInk
                                    font.pixelSize: 13
                                    font.weight: Font.DemiBold
                                }
                            }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.unitsDone < page.units.length
                    text: "You can take it before finishing them all; the questions come from all five."
                    color: Theme.faint
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Card {
                Layout.fillWidth: true
                title: "How it is marked"
                icon: "target"

                Repeater {
                    model: ["15 questions, about 10 minutes", "No second tries", "70% passes",
                            "Misses point back to their lessons", "Retake it as often as you like"]

                    delegate: RowLayout {
                        required property string modelData
                        Layout.fillWidth: true
                        spacing: 8

                        Rectangle {
                            implicitWidth: 6
                            implicitHeight: 6
                            radius: 3
                            color: Theme.accent
                        }
                        Text {
                            Layout.fillWidth: true
                            text: parent.modelData
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }
    }
}
