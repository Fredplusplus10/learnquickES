import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Every trainer of the plan in one place, to practise whenever the learner likes. A round still ticks today's
// task when the plan asks for it. Beside the trainers: what today's plan asks for and what the coach suggests.
// Each trainer shows the learner's first-try answers in its skill over the coach's two weeks.
ScrollPage {
    id: page

    required property LearnerController learner
    required property TodayModel today
    maxWidth: 1240

    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 960
    readonly property int tileColumns: columnWidth >= 760 ? 2 : 1

    // `skill` is the skill of lq's answer tallies the trainer's answers count towards; empty when its items are
    // self-checked and have no score.
    readonly property var groups: [
        { title: "Every day", detail: "Your review pile and a bit of everything", trainers: [
            { id: "review", icon: "cards", skill: "review" },
            { id: "mixed", icon: "sparkles", skill: "grammar" }
        ] },
        { title: "Grammar", detail: "The verbs, tenses and genders that carry most sentences", trainers: [
            { id: "verbs", icon: "bolt", skill: "verbs" },
            { id: "past", icon: "clock", skill: "past" },
            { id: "gender", icon: "book", skill: "gender" }
        ] },
        { title: "Words", detail: "English you already know, and the words that only look like it", trainers: [
            { id: "cognates", icon: "lightbulb", skill: "cognates" },
            { id: "falsefriends", icon: "eye", skill: "falsefriends" }
        ] },
        { title: "Speaking and describing", detail: "Out loud, checked by you", trainers: [
            { id: "speak", icon: "mic", skill: "" },
            { id: "describe", icon: "pen", skill: "" }
        ] }
    ]

    // Not `data`: every Item already has a property of that name.
    property var report: learner.coachReport()
    Connections {
        target: page.learner
        function onChanged() { page.report = page.learner.coachReport() }
    }

    function skill(id) {
        return (report.skills ?? []).find(s => s.id === id) ?? null
    }
    // Today's plan tasks that are a practice round, with whether each is done.
    readonly property var planRounds: today.tasks.filter(task => (task.route ?? "").startsWith("practice/"))
    function planTask(trainerId) {
        return planRounds.find(task => task.route === "practice/" + trainerId) ?? null
    }
    // The coach's first suggestion that opens a trainer, the review pile last (it has its own banner).
    readonly property var coachPick: {
        const tips = learner.coachTips.filter(tip => (tip.route ?? "").startsWith("practice/"))
        return tips.find(tip => tip.route !== "practice/review") ?? tips[0] ?? null
    }
    function coachPicked(trainerId) {
        return coachPick !== null && coachPick.route === "practice/" + trainerId
    }

    // A small rounded label.
    component Badge: Rectangle {
        id: badge
        property string text
        property color fill: Theme.accentSoft
        property color ink: Theme.accentStrong
        property string icon
        implicitWidth: badgeRow.implicitWidth + 14
        implicitHeight: 22
        radius: 11
        color: fill

        RowLayout {
            id: badgeRow
            anchors.centerIn: parent
            spacing: 4
            Icon {
                visible: badge.icon !== ""
                name: badge.icon
                size: 12
                color: badge.ink
            }
            Text {
                text: badge.text
                color: badge.ink
                font.pixelSize: 11
                font.weight: Font.DemiBold
            }
        }
    }

    // A link with a chevron, as on the coach's screen.
    component ActionLink: AbstractButton {
        id: link
        property string route
        focusPolicy: Qt.StrongFocus
        onClicked: page.openRoute(route)
        Keys.onReturnPressed: clicked()
        Keys.onEnterPressed: clicked()
        Accessible.name: text
        background: null
        contentItem: RowLayout {
            spacing: 2
            Text {
                text: link.text
                color: link.hovered || link.visualFocus ? Theme.accentStrong : Theme.accent
                font.pixelSize: 14
                font.weight: Font.DemiBold
                font.underline: link.visualFocus
            }
            Icon {
                name: "chevron-right"
                size: 14
                color: link.hovered || link.visualFocus ? Theme.accentStrong : Theme.accent
            }
        }
    }

    PageHero {
        overline: "Trainers"
        title: "Practice"
        subtitle: "Every trainer of the plan, whenever you like. A round ticks today's task when the plan asks for it, "
                  + "and misses come back in a second try at the end."
    }

    // The review pile, the first thing worth doing.
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: pileRow.implicitHeight + 30
        radius: Theme.radius
        color: page.learner.dueCount > 0 ? Theme.accentSoft : Theme.panel
        border.color: page.learner.dueCount > 0 ? Theme.accentLine : Theme.line

        RowLayout {
            id: pileRow
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 20 }
            spacing: 14

            Icon {
                name: page.learner.dueCount > 0 ? "cards" : "check"
                color: page.learner.dueCount > 0 ? Theme.accent : Theme.success
                size: 26
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: page.learner.deckSize === 0 ? "Your review deck is empty for now"
                          : page.learner.dueCount === 0 ? "Your review pile is clear"
                          : page.learner.dueCount === 1 ? "1 card due for review"
                          : page.learner.dueCount + " cards due for review"
                    color: Theme.ink
                    font.pixelSize: 17
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: page.learner.deckSize === 0 ? "Words join it when you finish their lesson."
                          : page.learner.dueCount === 0 ? "All " + page.learner.deckSize + " words of your deck are up to date. Any trainer below is extra practice."
                          : "Words come back just before you forget them, so a few minutes now saves relearning them later."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
            PrimaryButton {
                visible: page.learner.dueCount > 0
                text: "Review now"
                onClicked: page.openRoute("practice/review")
            }
            PrimaryButton {
                visible: page.learner.deckSize === 0
                secondary: true
                text: "Open lessons"
                onClicked: page.openRoute("lessons")
            }
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        // The trainers, by what they work on.
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Repeater {
                model: page.groups

                delegate: Card {
                    id: group
                    required property var modelData
                    Layout.fillWidth: true
                    title: modelData.title
                    subtitle: page.columnWidth >= 760 ? modelData.detail : ""

                    GridLayout {
                        Layout.fillWidth: true
                        columns: page.tileColumns
                        rowSpacing: 12
                        columnSpacing: 12

                        Repeater {
                            model: group.modelData.trainers

                            delegate: AbstractButton {
                                id: tile
                                required property var modelData
                                readonly property var info: page.learner.trainer(modelData.id)
                                readonly property var stats: modelData.skill === "" ? null : page.skill(modelData.skill)
                                readonly property var task: page.planTask(modelData.id)
                                readonly property bool picked: page.coachPicked(modelData.id)

                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                Layout.fillHeight: true
                                implicitHeight: tileColumn.implicitHeight + 32
                                focusPolicy: Qt.StrongFocus
                                onClicked: page.openRoute("practice/" + modelData.id)
                                Keys.onReturnPressed: clicked()
                                Keys.onEnterPressed: clicked()
                                Accessible.name: (info.title ?? "") + ". " + (info.description ?? "")

                                background: Rectangle {
                                    radius: Theme.smallRadius + 2
                                    color: tile.hovered ? Theme.hover : Theme.raised
                                    border.color: tile.visualFocus ? Theme.accent : tile.hovered ? Theme.hoverLine : Theme.line
                                    border.width: tile.visualFocus ? 2 : 1
                                }
                                contentItem: Item {
                                    ColumnLayout {
                                        id: tileColumn
                                        anchors { fill: parent; margins: 16 }
                                        spacing: 10

                                        RowLayout {
                                            Layout.fillWidth: true
                                            spacing: 12

                                            Rectangle {
                                                Layout.alignment: Qt.AlignTop
                                                implicitWidth: 40
                                                implicitHeight: 40
                                                radius: 20
                                                color: Theme.accentSoft

                                                Icon {
                                                    anchors.centerIn: parent
                                                    name: tile.modelData.icon
                                                    size: 20
                                                    color: Theme.accentStrong
                                                }
                                            }
                                            ColumnLayout {
                                                Layout.fillWidth: true
                                                spacing: 3

                                                Text {
                                                    Layout.fillWidth: true
                                                    text: tile.info.title ?? ""
                                                    color: Theme.ink
                                                    font.pixelSize: 16
                                                    font.weight: Font.DemiBold
                                                    wrapMode: Text.WordWrap
                                                    textFormat: Text.PlainText
                                                }
                                                Text {
                                                    Layout.fillWidth: true
                                                    text: tile.info.description ?? ""
                                                    color: Theme.muted
                                                    font.pixelSize: 13
                                                    wrapMode: Text.WordWrap
                                                    textFormat: Text.PlainText
                                                }
                                            }
                                            Icon {
                                                Layout.alignment: Qt.AlignVCenter
                                                name: "chevron-right"
                                                size: 18
                                                color: tile.hovered || tile.visualFocus ? Theme.accentStrong : Theme.faint
                                            }
                                        }
                                        Flow {
                                            Layout.fillWidth: true
                                            Layout.leftMargin: 52
                                            spacing: 6
                                            visible: dueBadge.visible || planBadge.visible || pickBadge.visible

                                            Badge {
                                                id: dueBadge
                                                visible: tile.modelData.id === "review" && page.learner.dueCount > 0
                                                text: page.learner.dueCount + " due"
                                                fill: Theme.accent
                                                ink: Theme.inkOnAccent
                                            }
                                            Badge {
                                                id: planBadge
                                                visible: tile.task !== null
                                                text: tile.task?.done ? "Done today" : "In today's plan"
                                                icon: tile.task?.done ? "check" : "calendar"
                                                fill: tile.task?.done ? Theme.successSoft : Theme.blueSoft
                                                ink: tile.task?.done ? Theme.goodInk : Theme.blue
                                            }
                                            Badge {
                                                id: pickBadge
                                                visible: tile.picked
                                                text: "Coach's pick"
                                                icon: "route"
                                            }
                                        }
                                        Item { Layout.fillHeight: true }
                                        // First-try answers in the trainer's skill, as the coach counts them.
                                        RowLayout {
                                            Layout.fillWidth: true
                                            Layout.leftMargin: 52
                                            spacing: 8

                                            Text {
                                                Layout.fillWidth: true
                                                text: {
                                                    const s = tile.stats
                                                    if (tile.modelData.skill === "")
                                                        return "Checked by you: no score"
                                                    if (s === null || s.total === 0)
                                                        return "No answers in the last " + (page.report.windowDays ?? 14) + " days"
                                                    if (!s.enough)
                                                        return s.label + " · " + s.total + (s.total === 1 ? " answer" : " answers")
                                                               + ", too few to judge"
                                                    return s.label + " · right first try, last " + (page.report.windowDays ?? 14) + " days"
                                                }
                                                color: Theme.muted
                                                font.pixelSize: 12
                                                wrapMode: Text.WordWrap
                                            }
                                            Text {
                                                visible: tile.stats !== null && tile.stats.enough
                                                text: (tile.stats?.accuracy ?? 0) + "%"
                                                color: (tile.stats?.accuracy ?? 0) >= (page.report.weakBelow ?? 70) ? Theme.goodInk : Theme.accentStrong
                                                font.pixelSize: 14
                                                font.weight: Font.Bold
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Today's plan and the coach's pick.
        ColumnLayout {
            Layout.fillWidth: !page.wide
            Layout.preferredWidth: page.wide ? 360 : -1
            Layout.maximumWidth: page.wide ? 360 : -1
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "Today's plan"
                subtitle: page.today.finished ? "" : "Day " + page.today.dayNumber
                icon: "calendar"

                Text {
                    Layout.fillWidth: true
                    visible: page.planRounds.length === 0
                    text: page.today.finished ? "You finished the plan. Every trainer is here for as long as you like."
                          : "Today's plan has no practice round. Anything here is extra, and counts towards your skills."
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
                Repeater {
                    model: page.planRounds

                    delegate: RowLayout {
                        id: round
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            implicitWidth: 22
                            implicitHeight: 22
                            radius: 11
                            color: round.modelData.done ? Theme.success : "transparent"
                            border.color: round.modelData.done ? Theme.success : Theme.edge
                            border.width: 2

                            Icon {
                                anchors.centerIn: parent
                                visible: round.modelData.done
                                name: "check"
                                size: 13
                                color: Theme.inkOnAccent
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1

                            Text {
                                Layout.fillWidth: true
                                text: round.modelData.name
                                color: Theme.ink
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                font.strikeout: round.modelData.done
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                            Text {
                                Layout.fillWidth: true
                                text: round.modelData.minutes + " min"
                                color: Theme.muted
                                font.pixelSize: 12
                            }
                        }
                        ActionLink {
                            visible: !round.modelData.done
                            text: "Start"
                            route: round.modelData.route
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.planRounds.length > 0
                    text: page.planRounds.every(task => task.done) ? "Done for today. More rounds still count towards your skills."
                          : "Finishing a round ticks it on Today."
                    color: Theme.faint
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Your coach suggests"
                icon: "route"

                Text {
                    Layout.fillWidth: true
                    visible: page.coachPick === null
                    text: "Nothing in particular: pick what you enjoy. Your coach's other suggestions are on its own screen."
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.coachPick !== null
                    text: page.coachPick?.title ?? ""
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.coachPick !== null
                    text: page.coachPick?.detail ?? ""
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    ActionLink {
                        visible: page.coachPick !== null
                        text: "Open practice"
                        route: page.coachPick?.route ?? ""
                    }
                    Item { Layout.fillWidth: true }
                    ActionLink {
                        text: "All suggestions"
                        route: "coach"
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                visible: page.learner.tasteText !== ""
                title: "Mixed practice leans your way"
                icon: "sparkles"

                Text {
                    Layout.fillWidth: true
                    text: page.learner.tasteText
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
            }
        }
    }
}
