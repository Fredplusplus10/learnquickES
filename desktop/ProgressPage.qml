import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts
import QtQuick.Shapes

// What the learner has done, from recorded activity only. On top: the goal and pace, the five phases, weekly
// Spanish time, the deck growing, the streak, the calendar and the latest achievements. Below, in detail: time
// in Spanish, lessons, words, streak, minutes per day, accuracy by skill, signs of B1, certificates and every
// achievement.
ScrollPage {
    id: page

    required property LearnerController learner
    required property TodayModel today
    required property ProfileController profile

    maxWidth: 1240

    property var report: learner.progressReport()
    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 960
    readonly property var weeks: report.weeks ?? []

    Connections {
        target: page.learner
        function onChanged() { page.report = page.learner.progressReport() }
    }

    component SummaryTile: Rectangle {
        id: tile
        property string label
        property string value
        property string detail
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.fillHeight: true  // one height for the row, set by the tallest
        implicitHeight: tileColumn.implicitHeight + 32
        radius: Theme.radius
        color: Theme.panel
        border.color: Theme.line

        ColumnLayout {
            id: tileColumn
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 }
            spacing: 4

            Text {
                text: tile.label
                color: Theme.muted
                font.pixelSize: 13
            }
            Text {
                text: tile.value
                color: Theme.ink
                font.pixelSize: 24
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                text: tile.detail
                color: Theme.muted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }
    }

    // One block of the goal strip: an icon, what it is, the value and a line under it.
    component GoalBlock: RowLayout {
        id: block
        property string icon
        property string label
        property string value
        property color valueColor: Theme.ink
        property string detail
        Layout.fillWidth: true
        Layout.preferredWidth: 1
        Layout.alignment: Qt.AlignTop
        spacing: 12

        Icon {
            Layout.alignment: Qt.AlignTop
            Layout.topMargin: 2
            name: block.icon
            color: Theme.accent
            size: 26
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: block.label
                color: Theme.ink
                font.pixelSize: 14
                font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true
                text: block.value
                color: block.valueColor
                font.pixelSize: 22
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                text: block.detail
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
        }
    }

    // A chart's empty state, keeping the card's size: a line that says why there is nothing to draw.
    component NoData: ColumnLayout {
        id: noData
        property string icon
        property string text
        property string detail
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.minimumHeight: 170
        spacing: 10

        Item { Layout.fillHeight: true }
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            implicitWidth: 48
            implicitHeight: 48
            radius: 24
            color: Theme.accentSoft

            Icon {
                anchors.centerIn: parent
                name: noData.icon
                color: Theme.accent
                size: 24
            }
        }
        Text {
            Layout.fillWidth: true
            text: noData.text
            color: Theme.ink
            font.pixelSize: 14
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            visible: noData.detail !== ""
            text: noData.detail
            color: Theme.muted
            font.pixelSize: 13
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }
        Item { Layout.fillHeight: true }
    }

    // Week labels under a chart: the current week always, and a few before it, so they never crowd.
    function weekLabelled(index) {
        const step = Math.max(1, Math.ceil(weeks.length / 4))
        return (weeks.length - 1 - index) % step === 0
    }

    PageHero {
        overline: "Progress"
        title: "Your progress"
        highlight: (page.report.lessons ?? 0) + " of " + (page.report.lessonCount ?? 0) + " lessons"
        detail: (page.report.wordsRemembered ?? 0) + " words remembered · " + (page.report.spanishHours ?? "0.0")
                + " h of Spanish"
        subtitle: "Your plan, your pace and what you have done so far. Everything here comes from what you have "
                  + "recorded: nothing is an estimate or a sample."
    }

    // ---- Goal, target date and pace ------------------------------------------------------------------------
    Card {
        Layout.fillWidth: true

        GridLayout {
            Layout.fillWidth: true
            columns: page.wide ? 5 : 1
            columnSpacing: 20
            rowSpacing: 16

            GoalBlock {
                icon: "target"
                label: "Your goal"
                value: "Finish the " + (page.report.goal?.days ?? 0) + "-day plan"
                detail: "Started " + (page.report.goal?.started ?? "") + " · 25 lessons in 5 phases, then the B1 mock exam"
            }
            Rectangle {
                visible: page.wide
                Layout.fillHeight: true
                implicitWidth: 1
                color: Theme.line
            }
            GoalBlock {
                icon: "calendar"
                label: "Target completion date"
                value: page.report.goal?.target || "Not set"
                detail: page.report.goal?.targetDetail ?? ""
            }
            Rectangle {
                visible: page.wide
                Layout.fillHeight: true
                implicitWidth: 1
                color: Theme.line
            }
            GoalBlock {
                readonly property string tone: page.report.pace?.tone ?? "none"
                icon: "bolt"
                label: "Current pace"
                value: page.report.pace?.text ?? ""
                valueColor: tone === "good" || tone === "done" ? Theme.goodInk
                          : tone === "behind" ? Theme.accentStrong : Theme.muted
                detail: page.report.pace?.detail ?? ""
            }
        }
    }

    // ---- The five phases -----------------------------------------------------------------------------------
    Card {
        Layout.fillWidth: true
        title: "Your study roadmap"
        subtitle: "Day ranges for your " + (page.report.goal?.days ?? 0) + "-day plan"
        icon: "route"

        GridLayout {
            id: roadmapGrid
            readonly property bool timeline: columns === 5
            readonly property var phases: page.report.roadmap ?? []
            Layout.fillWidth: true
            columns: page.columnWidth >= 900 ? 5 : 1
            columnSpacing: 12
            rowSpacing: 12

            Repeater {
                model: roadmapGrid.phases

                delegate: ColumnLayout {
                    id: phase
                    required property var modelData
                    required property int index
                    readonly property bool done: modelData.state === "done"
                    readonly property bool current: modelData.state === "current"
                    readonly property bool reached: done || current
                    readonly property bool nextReached: index + 1 < roadmapGrid.phases.length
                                                        && roadmapGrid.phases[index + 1].state !== "upcoming"
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    spacing: 10

                    // The timeline: a dot per phase, joined by a line that fills as phases are reached.
                    Item {
                        visible: roadmapGrid.timeline
                        Layout.fillWidth: true
                        implicitHeight: 26

                        Rectangle {
                            visible: phase.index > 0
                            x: -roadmapGrid.columnSpacing / 2
                            width: parent.width / 2 + roadmapGrid.columnSpacing / 2
                            anchors.verticalCenter: parent.verticalCenter
                            height: 3
                            color: phase.reached ? Theme.accent : Theme.line
                        }
                        Rectangle {
                            visible: phase.index < roadmapGrid.phases.length - 1
                            x: parent.width / 2
                            width: parent.width / 2 + roadmapGrid.columnSpacing / 2
                            anchors.verticalCenter: parent.verticalCenter
                            height: 3
                            color: phase.nextReached ? Theme.accent : Theme.line
                        }
                        Rectangle {
                            anchors.centerIn: parent
                            width: 26
                            height: 26
                            radius: 13
                            color: phase.done ? Theme.accent : Theme.panel
                            border.color: phase.reached ? Theme.accent : Theme.edge
                            border.width: 2.5

                            Icon {
                                anchors.centerIn: parent
                                visible: phase.done
                                name: "check"
                                color: Theme.inkOnAccent
                                size: 15
                            }
                            Rectangle {
                                anchors.centerIn: parent
                                visible: phase.current
                                width: 10
                                height: 10
                                radius: 5
                                color: Theme.accent
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        implicitHeight: phaseColumn.implicitHeight + 28
                        radius: Theme.smallRadius
                        color: phase.current ? Theme.accentSoft : Theme.raised
                        border.color: phase.current ? Theme.accentLine : Theme.line
                        Accessible.role: Accessible.Grouping
                        Accessible.name: "Phase " + phase.modelData.n + ", " + phase.modelData.title + ": "
                                         + (phase.done ? "completed" : phase.current ? "in progress" : "upcoming")

                        ColumnLayout {
                            id: phaseColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 6

                            Text {
                                Layout.fillWidth: true
                                text: phase.modelData.n + ". " + phase.modelData.title
                                textFormat: Text.PlainText
                                color: Theme.ink
                                font.pixelSize: 15
                                font.weight: Font.Bold
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                Layout.fillWidth: true
                                text: phase.modelData.description
                                textFormat: Text.PlainText
                                color: Theme.muted
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                            }
                            Item { Layout.fillHeight: true }
                            RowLayout {
                                spacing: 8

                                Rectangle {
                                    implicitWidth: pillRow.implicitWidth + 20
                                    implicitHeight: 26
                                    radius: 13
                                    color: phase.done ? Theme.successSoft : phase.current ? Theme.panel : Theme.background
                                    border.color: phase.done ? Theme.goodLine : phase.current ? Theme.accentLine : Theme.line

                                    RowLayout {
                                        id: pillRow
                                        anchors.centerIn: parent
                                        spacing: 5

                                        Icon {
                                            name: phase.done ? "check" : phase.current ? "play" : "clock"
                                            size: 13
                                            color: phase.done ? Theme.goodInk : phase.current ? Theme.accentStrong : Theme.muted
                                        }
                                        Text {
                                            text: phase.done ? "Completed" : phase.current ? "In progress" : "Upcoming"
                                            color: phase.done ? Theme.goodInk : phase.current ? Theme.accentStrong : Theme.muted
                                            font.pixelSize: 12
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                }
                                Text {
                                    text: phase.modelData.days
                                    color: Theme.muted
                                    font.pixelSize: 12
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Three trends: weekly time, the deck, pronunciation ------------------------------------------------
    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 3 : 1
        columnSpacing: 18
        rowSpacing: 18

        Card {
            id: weeklyCard
            readonly property int most: page.report.mostWeekMinutes ?? 0
            // Whole hours for the scale's top, so the tallest bar never touches it.
            readonly property int scaleHours: Math.max(1, Math.ceil(most / 60))
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            title: "Weekly Spanish time"
            subtitle: most > 0 ? page.report.weekAverage + " h a week" : ""
            icon: "clock"

            Text {
                Layout.fillWidth: true
                text: "Hours a week, Monday to Sunday: app time, listening, the island and conversations."
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            Item {
                id: weeklyPlot
                visible: weeklyCard.most > 0
                Layout.fillWidth: true
                implicitHeight: 176

                readonly property real plotTop: 20
                readonly property real plotHeight: 124
                readonly property real axisY: plotTop + plotHeight
                readonly property real slot: width / Math.max(1, page.weeks.length)

                Rectangle {
                    y: weeklyPlot.plotTop
                    width: parent.width
                    height: 1
                    color: Theme.line
                    opacity: 0.6
                }
                Text {
                    y: weeklyPlot.plotTop - implicitHeight - 2
                    text: weeklyCard.scaleHours + " h"
                    color: Theme.faint
                    font.pixelSize: 11
                }
                Rectangle {
                    y: weeklyPlot.axisY
                    width: parent.width
                    height: 1
                    color: Theme.edge
                }
                Repeater {
                    model: page.weeks

                    delegate: Item {
                        id: weekBar
                        required property var modelData
                        required property int index
                        readonly property real barHeight: weeklyPlot.plotHeight * modelData.minutes / (weeklyCard.scaleHours * 60)
                        x: index * weeklyPlot.slot
                        width: weeklyPlot.slot
                        height: weeklyPlot.height

                        Rectangle {
                            anchors.fill: parent
                            anchors.bottomMargin: weeklyPlot.height - weeklyPlot.axisY
                            color: weekHit.containsMouse ? Theme.hover : "transparent"
                            radius: 4
                        }
                        Rectangle {
                            visible: weekBar.modelData.minutes > 0
                            x: (parent.width - width) / 2
                            y: weeklyPlot.axisY - height
                            width: Math.min(26, weeklyPlot.slot - 6)
                            height: Math.max(3, weekBar.barHeight)
                            radius: Math.min(4, height / 2)
                            color: weekHit.containsMouse || weekBar.modelData.current ? Theme.accentStrong : Theme.accent

                            Rectangle {
                                anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                                height: Math.min(parent.height, 4)
                                color: parent.color
                            }
                        }
                        Text {
                            visible: page.weekLabelled(weekBar.index)
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: weeklyPlot.axisY + 6
                            text: weekBar.modelData.current ? "This week" : weekBar.modelData.label
                            color: weekBar.modelData.current ? Theme.ink : Theme.muted
                            font.pixelSize: 11
                            font.weight: weekBar.modelData.current ? Font.DemiBold : Font.Normal
                        }
                        MouseArea {
                            id: weekHit
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                        }
                        ToolTip.visible: weekHit.containsMouse
                        ToolTip.text: weekBar.modelData.range + ": " + (weekBar.modelData.minutes / 60).toFixed(1) + " h ("
                                      + weekBar.modelData.minutes + " min)"
                        ToolTip.delay: 150
                    }
                }
            }
            NoData {
                visible: weeklyCard.most === 0
                icon: "clock"
                text: "No Spanish time recorded yet."
                detail: "Lessons, practice and logged listening all count."
            }
        }

        Card {
            id: wordsCard
            readonly property int total: page.report.deckWords ?? 0
            readonly property int scaleTop: Math.max(10, Math.ceil(total / 50) * 50)
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            title: "Words in your deck"
            subtitle: total > 0 ? (total === 1 ? "1 word" : total + " words") : ""
            icon: "cards"

            Text {
                Layout.fillWidth: true
                text: "Each lesson adds its words to your review deck. The deck at the end of each week."
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            Item {
                id: wordsPlot
                visible: wordsCard.total > 0
                Layout.fillWidth: true
                implicitHeight: 176

                readonly property real plotTop: 20
                readonly property real plotHeight: 124
                readonly property real axisY: plotTop + plotHeight
                readonly property real slot: width / Math.max(1, page.weeks.length)
                function pointAt(i) {
                    return Qt.point((i + 0.5) * slot, axisY - plotHeight * page.weeks[i].words / wordsCard.scaleTop)
                }
                readonly property var points: {
                    const list = []
                    for (let i = 0; i < page.weeks.length; ++i)
                        list.push(pointAt(i))
                    return list
                }

                Rectangle {
                    y: wordsPlot.plotTop
                    width: parent.width
                    height: 1
                    color: Theme.line
                    opacity: 0.6
                }
                Text {
                    y: wordsPlot.plotTop - implicitHeight - 2
                    text: wordsCard.scaleTop + " words"
                    color: Theme.faint
                    font.pixelSize: 11
                }
                Rectangle {
                    y: wordsPlot.axisY
                    width: parent.width
                    height: 1
                    color: Theme.edge
                }
                // The area under the line, then the line.
                Shape {
                    anchors.fill: parent
                    visible: wordsPlot.points.length > 1
                    preferredRendererType: Shape.CurveRenderer
                    Accessible.ignored: true

                    ShapePath {
                        strokeColor: "transparent"
                        fillColor: Theme.accentSoft
                        startX: wordsPlot.points.length > 0 ? wordsPlot.points[0].x : 0
                        startY: wordsPlot.axisY
                        PathPolyline {
                            path: wordsPlot.points.length > 0
                                  ? [Qt.point(wordsPlot.points[0].x, wordsPlot.axisY)].concat(wordsPlot.points)
                                    .concat([Qt.point(wordsPlot.points[wordsPlot.points.length - 1].x, wordsPlot.axisY)])
                                  : []
                        }
                    }
                    ShapePath {
                        strokeColor: Theme.accent
                        strokeWidth: 2.5
                        fillColor: "transparent"
                        joinStyle: ShapePath.RoundJoin
                        capStyle: ShapePath.RoundCap
                        PathPolyline { path: wordsPlot.points }
                    }
                }
                Repeater {
                    model: page.weeks

                    delegate: Item {
                        id: wordPoint
                        required property var modelData
                        required property int index
                        readonly property point at: wordsPlot.pointAt(index)
                        x: index * wordsPlot.slot
                        width: wordsPlot.slot
                        height: wordsPlot.height

                        Rectangle {
                            x: wordPoint.at.x - wordPoint.x - width / 2
                            y: wordPoint.at.y - height / 2
                            width: wordHit.containsMouse || wordPoint.modelData.current ? 11 : 8
                            height: width
                            radius: width / 2
                            color: Theme.panel
                            border.color: Theme.accent
                            border.width: 2.5
                        }
                        Text {
                            visible: wordPoint.modelData.current
                            x: Math.min(parent.width - implicitWidth, Math.max(-wordPoint.x, wordPoint.at.x - wordPoint.x - implicitWidth / 2))
                            y: wordPoint.at.y - implicitHeight - 9
                            text: wordPoint.modelData.words
                            color: Theme.ink
                            font.pixelSize: 11
                            font.weight: Font.DemiBold
                        }
                        Text {
                            visible: page.weekLabelled(wordPoint.index)
                            anchors.horizontalCenter: parent.horizontalCenter
                            y: wordsPlot.axisY + 6
                            text: wordPoint.modelData.current ? "This week" : wordPoint.modelData.label
                            color: wordPoint.modelData.current ? Theme.ink : Theme.muted
                            font.pixelSize: 11
                            font.weight: wordPoint.modelData.current ? Font.DemiBold : Font.Normal
                        }
                        MouseArea {
                            id: wordHit
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                        }
                        ToolTip.visible: wordHit.containsMouse
                        ToolTip.text: "End of " + wordPoint.modelData.range + ": " + wordPoint.modelData.words + " words"
                        ToolTip.delay: 150
                    }
                }
            }
            NoData {
                visible: wordsCard.total === 0
                icon: "cards"
                text: "Your deck is empty for now."
                detail: "Finish a lesson and its words join your deck."
            }
        }

        // No verified pronunciation scores exist, so there is no trend to draw: the card says so.
        Card {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            title: "Pronunciation"
            subtitle: "No scores yet"
            icon: "mic"

            Text {
                Layout.fillWidth: true
                text: "A trend of your pronunciation scores, once LearnQuick can measure them."
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            NoData {
                readonly property int spoken: page.report.spoken ?? 0
                icon: "mic"
                text: "LearnQuick doesn't score pronunciation yet, so there is no trend to show."
                detail: spoken === 0 ? "Phrases you say out loud in lessons are counted here."
                      : spoken === 1 ? "You have said 1 phrase out loud in lessons."
                      : "You have said " + spoken + " phrases out loud in lessons."
            }
        }
    }

    // ---- Streak, calendar and the latest achievements ------------------------------------------------------
    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 3 : 1
        columnSpacing: 18
        rowSpacing: 18

        StreakCard {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            today: page.today
            profile: page.profile
            canChangeIcon: false
        }
        CalendarCard {
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            today: page.today
        }
        Card {
            id: latestCard
            // The earned ones first, then the next ones to earn: four in all.
            readonly property var shown: {
                const all = page.report.achievements ?? []
                return all.filter(a => a.earned).concat(all.filter(a => !a.earned)).slice(0, 4)
            }
            Layout.fillWidth: true
            Layout.preferredWidth: 1
            Layout.fillHeight: true
            title: "Achievements"
            subtitle: (page.report.earnedCount ?? 0) + " of " + (page.report.achievements ?? []).length
            icon: "award"

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Repeater {
                    model: latestCard.shown

                    delegate: ColumnLayout {
                        id: latest
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.alignment: Qt.AlignTop
                        spacing: 6
                        Accessible.role: Accessible.StaticText
                        Accessible.name: modelData.title + (modelData.earned ? ", earned " + modelData.when : ", locked")

                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            implicitWidth: 46
                            implicitHeight: 46
                            radius: 23
                            color: latest.modelData.earned ? Theme.accent : Theme.line

                            Icon {
                                anchors.centerIn: parent
                                name: latest.modelData.earned ? latest.modelData.icon : "lock"
                                size: 21
                                color: latest.modelData.earned ? Theme.inkOnAccent : Theme.faint
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            text: latest.modelData.title
                            color: latest.modelData.earned ? Theme.ink : Theme.muted
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                            elide: Text.ElideRight
                        }
                    }
                }
            }
            Item { Layout.fillHeight: true }
            AbstractButton {
                id: seeAll
                Layout.alignment: Qt.AlignRight
                focusPolicy: Qt.StrongFocus
                onClicked: page.scrollTo(allAchievements)
                Accessible.name: "See all achievements"
                contentItem: RowLayout {
                    spacing: 2
                    Text {
                        text: "See all"
                        color: seeAll.hovered || seeAll.visualFocus ? Theme.accentStrong : Theme.accent
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        font.underline: seeAll.visualFocus
                    }
                    Icon {
                        name: "chevron-right"
                        size: 14
                        color: seeAll.hovered || seeAll.visualFocus ? Theme.accentStrong : Theme.accent
                    }
                }
                background: null
            }
        }
    }

    // ---- In detail: the numbers behind it all --------------------------------------------------------------
    ColumnLayout {
        Layout.fillWidth: true
        Layout.topMargin: 10
        spacing: 2

        Text {
            text: "In detail"
            color: Theme.ink
            font.pixelSize: 22
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            text: "Totals, the last two weeks day by day, accuracy by skill and every achievement."
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.width > 760 ? 4 : 2
        rowSpacing: 12
        columnSpacing: 12

        SummaryTile {
            label: "Time in Spanish"
            value: page.report.spanishHours + " h"
            detail: page.report.spanishDetail
        }
        SummaryTile {
            label: "Lessons"
            value: page.report.lessons + " / " + page.report.lessonCount
            detail: page.report.planDone + " of " + page.report.planTotal + " plan days done"
        }
        SummaryTile {
            label: "Words"
            value: page.report.wordsRemembered + " / " + page.report.wordsSeen
            detail: "remembered / in your deck"
        }
        SummaryTile {
            label: "Streak"
            value: page.report.streak + (page.report.streak === 1 ? " day" : " days")
            detail: "best " + page.report.longest + " · " + page.report.freezes
                    + (page.report.freezes === 1 ? " freeze" : " freezes")
        }
    }

    Card {
        id: chartCard
        Layout.fillWidth: true
        title: "Minutes in Spanish per day"
        subtitle: "Last 14 days"

        readonly property var days: page.report.minutes ?? []
        readonly property int most: page.report.mostMinutes ?? 0
        // A round top for the scale, so the tallest bar never touches it.
        readonly property int scaleTop: Math.max(30, Math.ceil(most / 30) * 30)

        Text {
            Layout.fillWidth: true
            text: "App time, listening, the language island and conversations. Point at a bar for its minutes."
            color: Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        Item {
            id: plot
            Layout.fillWidth: true
            implicitHeight: 190
            visible: chartCard.most > 0

            readonly property real plotTop: 22
            readonly property real plotHeight: 140
            readonly property real axisY: plotTop + plotHeight
            readonly property real slot: width / Math.max(1, chartCard.days.length)
            readonly property real barWidth: Math.min(28, slot - 6)

            // The scale's top, as one quiet line with its value.
            Rectangle {
                y: plot.plotTop
                width: parent.width
                height: 1
                color: Theme.line
                opacity: 0.6
            }
            Text {
                y: plot.plotTop - implicitHeight - 2
                text: chartCard.scaleTop + " min"
                color: Theme.faint
                font.pixelSize: 11
            }
            Rectangle {
                y: plot.axisY
                width: parent.width
                height: 1
                color: Theme.edge
            }

            Repeater {
                model: chartCard.days

                delegate: Item {
                    id: bar
                    required property var modelData
                    required property int index
                    readonly property real barHeight: plot.plotHeight * modelData.minutes / chartCard.scaleTop
                    readonly property bool labelled: modelData.minutes > 0
                                                     && (modelData.today || modelData.minutes === chartCard.most)
                    x: index * plot.slot
                    width: plot.slot
                    height: plot.height

                    Rectangle {
                        anchors.fill: parent
                        anchors.bottomMargin: plot.height - plot.axisY
                        color: hit.containsMouse ? Theme.hover : "transparent"
                        radius: 4
                    }
                    // Rounded at the top only: the square foot sits on the baseline.
                    Rectangle {
                        visible: bar.modelData.minutes > 0
                        x: (parent.width - width) / 2
                        y: plot.axisY - height
                        width: plot.barWidth
                        height: Math.max(3, bar.barHeight)
                        radius: Math.min(4, height / 2)
                        color: hit.containsMouse ? Theme.accentStrong : Theme.accent

                        Rectangle {
                            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
                            height: Math.min(parent.height, 4)
                            color: parent.color
                        }
                    }
                    Text {
                        visible: bar.labelled
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: plot.axisY - Math.max(3, bar.barHeight) - implicitHeight - 3
                        text: bar.modelData.minutes
                        color: Theme.ink
                        font.pixelSize: 11
                        font.weight: Font.DemiBold
                    }
                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        y: plot.axisY + 6
                        text: bar.modelData.day
                        color: bar.modelData.today ? Theme.ink : Theme.muted
                        font.pixelSize: 12
                        font.weight: bar.modelData.today ? Font.Bold : Font.Normal
                    }
                    MouseArea {
                        id: hit
                        anchors.fill: parent
                        hoverEnabled: true
                        acceptedButtons: Qt.NoButton
                    }
                    ToolTip.visible: hit.containsMouse
                    ToolTip.text: bar.modelData.label + (bar.modelData.today ? " (today)" : "") + ": "
                                  + bar.modelData.minutes + " min"
                    ToolTip.delay: 150
                }
            }
        }
        Text {
            visible: chartCard.most === 0
            Layout.fillWidth: true
            text: "No Spanish time in the last two weeks yet. Lessons, practice and logged listening all count."
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Accuracy by skill"
        subtitle: "Right on the first try, all time"

        Repeater {
            model: page.report.skills

            delegate: RowLayout {
                id: skill
                required property var modelData
                readonly property bool known: modelData.accuracy >= 0
                Layout.fillWidth: true
                spacing: 12

                Text {
                    Layout.preferredWidth: 110
                    text: skill.modelData.label
                    color: Theme.ink
                    font.pixelSize: 14
                }
                Rectangle {
                    visible: skill.known
                    Layout.fillWidth: true
                    implicitHeight: 8
                    radius: 4
                    color: Theme.line

                    Rectangle {
                        width: parent.width * skill.modelData.accuracy / 100
                        height: parent.height
                        radius: 4
                        color: Theme.accent
                    }
                }
                Text {
                    visible: skill.known
                    Layout.preferredWidth: 42
                    horizontalAlignment: Text.AlignRight
                    text: skill.modelData.accuracy + "%"
                    color: Theme.ink
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Text {
                    visible: skill.known
                    Layout.preferredWidth: 78
                    text: skill.modelData.answered === 1 ? "1 answer" : skill.modelData.answered + " answers"
                    color: Theme.faint
                    font.pixelSize: 12
                }
                Text {
                    visible: !skill.known
                    Layout.fillWidth: true
                    text: "no answers yet"
                    color: Theme.faint
                    font.pixelSize: 13
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Signs of B1"
        subtitle: page.report.readyCount + " of " + (page.report.readiness ?? []).length

        Repeater {
            model: page.report.readiness

            delegate: RowLayout {
                id: sign
                required property var modelData
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    implicitWidth: 22
                    implicitHeight: 22
                    radius: 11
                    color: sign.modelData.met ? Theme.success : "transparent"
                    border.color: sign.modelData.met ? Theme.success : Theme.faint
                    border.width: 2

                    Icon {
                        anchors.centerIn: parent
                        visible: sign.modelData.met
                        name: "check"
                        size: 13
                        color: Theme.inkOnAccent
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: sign.modelData.text
                    color: sign.modelData.met ? Theme.ink : Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
            }
        }
        Text {
            Layout.fillWidth: true
            Layout.topMargin: 4
            text: "A guide to a working B1, not a certificate: the official exam is the DELE B1 from the Instituto Cervantes."
            color: Theme.faint
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    // The A1, A2 and B1 Certificates of Readiness: earned ones open, the rest stay locked.
    Card {
        Layout.fillWidth: true
        title: "Certificates of Readiness"
        subtitle: "Not official CEFR qualifications"
        icon: "award"

        RowLayout {
            Layout.fillWidth: true
            spacing: 14

            Repeater {
                model: Certificates.levels

                delegate: Image {
                    required property var modelData
                    Layout.preferredWidth: 46
                    Layout.preferredHeight: 51
                    source: "assets/certificates/badges/level_" + modelData.level + ".svg"
                    sourceSize.width: 92
                    sourceSize.height: 102
                    fillMode: Image.PreserveAspectFit
                    opacity: modelData.earned ? 1 : 0.35
                    Accessible.name: "Spanish " + modelData.level + (modelData.earned ? ", earned" : ", locked")
                }
            }
            Text {
                Layout.fillWidth: true
                text: {
                    const earned = Certificates.levels.filter(entry => entry.earned)
                    return earned.length > 0
                        ? "Earned: " + earned.map(entry => entry.level + " on " + entry.issuedOn).join(", ") + "."
                        : "Each one is awarded after that level's own assessment, never for days or lessons alone."
                }
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            AbstractButton {
                id: certificatesLink
                focusPolicy: Qt.StrongFocus
                onClicked: page.openRoute("certificates")
                Accessible.name: "Open certificates"
                contentItem: RowLayout {
                    spacing: 2
                    Text {
                        text: "Open"
                        color: certificatesLink.hovered || certificatesLink.visualFocus ? Theme.accentStrong : Theme.accent
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        font.underline: certificatesLink.visualFocus
                    }
                    Icon {
                        name: "chevron-right"
                        size: 14
                        color: certificatesLink.hovered || certificatesLink.visualFocus ? Theme.accentStrong : Theme.accent
                    }
                }
                background: null
            }
        }
    }

    Card {
        id: allAchievements
        Layout.fillWidth: true
        title: "Achievements"
        subtitle: page.report.earnedCount + " of " + (page.report.achievements ?? []).length

        GridLayout {
            Layout.fillWidth: true
            columns: page.width > 820 ? 3 : page.width > 560 ? 2 : 1
            rowSpacing: 10
            columnSpacing: 10

            Repeater {
                model: page.report.achievements

                delegate: Rectangle {
                    id: badge
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    implicitHeight: badgeRow.implicitHeight + 24
                    radius: Theme.smallRadius
                    color: modelData.earned ? Theme.accentSoft : Theme.raised
                    border.color: modelData.earned ? Theme.accentLine : Theme.edge

                    RowLayout {
                        id: badgeRow
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
                        spacing: 12

                        Rectangle {
                            Layout.alignment: Qt.AlignTop
                            implicitWidth: 38
                            implicitHeight: 38
                            radius: 19
                            color: badge.modelData.earned ? Theme.accent : Theme.line

                            Icon {
                                anchors.centerIn: parent
                                name: badge.modelData.earned ? badge.modelData.icon : "lock"
                                size: 19
                                color: badge.modelData.earned ? Theme.inkOnAccent : Theme.faint
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                Layout.fillWidth: true
                                text: badge.modelData.title
                                color: badge.modelData.earned ? Theme.ink : Theme.muted
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                Layout.fillWidth: true
                                text: badge.modelData.earned ? "Earned " + badge.modelData.when : badge.modelData.how
                                color: badge.modelData.earned ? Theme.accentStrong : Theme.muted
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }
    }
}
