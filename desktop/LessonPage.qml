import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// One lesson, as on the web app: words, grammar hack, dialogue, practice, speaking, summary.
// The lesson is saved, and its words join the review deck, when the summary opens.
ScrollPage {
    id: page

    required property LearnerController learner
    required property SessionModel session
    property int unitIndex: 0
    property string initialStep: "vocab"

    readonly property var unit: learner.unit(unitIndex)
    readonly property var steps: [
        { id: "vocab", label: "Words" },
        { id: "grammar", label: "Grammar" },
        { id: "dialogue", label: "Dialogue" },
        { id: "practice", label: "Practice" },
        { id: "speak", label: "Speak" },
        { id: "done", label: "Done" }
    ]
    property int step: Math.max(0, steps.findIndex(s => s.id === initialStep))
    property int reached: step
    readonly property string stepId: steps[step].id

    property var practiceResult: null
    property var said: []  // phrases of the speaking step said out loud
    property var saved: null
    property var nextStep: ({})

    readonly property bool canNext: stepId !== "done" && (stepId !== "practice" || practiceResult !== null)

    maxWidth: 1240

    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    // The lesson goals and its words sit beside the step when there is room, below it otherwise.
    readonly property bool wide: columnWidth >= 1000
    // What each step is for, ticked as the learner gets past it (practice: once it has a score).
    readonly property var goals: [
        { step: 0, label: "Learn " + (unit.vocab ?? []).length + " words" },
        { step: 1, label: "Understand the grammar hack" },
        { step: 2, label: "Follow a real dialogue" },
        { step: 3, label: "Practise with exercises" },
        { step: 4, label: "Say it out loud" }
    ]
    function goalDone(goal) {
        return goal.step === 3 ? practiceResult !== null : reached > goal.step
    }

    function go(index) {
        Speech.stop()
        step = index
        reached = Math.max(reached, index)
        scrollToTop()
    }

    onStepIdChanged: enterStep()
    Component.onCompleted: enterStep()

    function enterStep() {
        if (stepId === "practice" && practiceResult === null && !session.active)
            session.startLesson(unitIndex)
        if (stepId === "done" && practiceResult !== null && saved === null) {
            saved = learner.finishLesson(unitIndex, practiceResult.score, said.length)
            nextStep = learner.nextStep("lesson")
        }
    }

    Connections {
        target: page.session
        function onCompleted() {
            if (page.stepId === "practice")
                page.practiceResult = page.session.result
        }
    }

    Keys.onReturnPressed: if (canNext) go(step + 1)
    Keys.onEnterPressed: if (canNext) go(step + 1)

    // Where the lesson sits: back to the course, and its phase.
    Breadcrumb {
        backLabel: "Lessons"
        trail: "Phase " + (page.unit.phase ?? "") + " · " + (page.unit.phaseTitle ?? "")
        onBack: page.openRoute("lessons")
    }

    PageHero {
        title: "Lesson " + (page.unit.n ?? "") + ": " + (page.unit.title ?? "")
        highlight: page.unit.trickName ?? ""
        detail: page.unit.titleEs ?? ""
        subtitle: page.unit.canDo ?? ""
    }

    // Steps: the ones reached can be revisited.
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: stepsColumn.implicitHeight + 28
        radius: Theme.radius
        color: Theme.panel
        border.color: Theme.line

        ColumnLayout {
            id: stepsColumn
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
            spacing: 10

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    Layout.fillWidth: true
                    text: page.stepId === "done" ? "Lesson finished"
                          : "Step " + (page.step + 1) + " of " + (page.steps.length - 1) + " · " + page.steps[page.step].label
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                Icon {
                    name: "clock"
                    size: 16
                    color: Theme.muted
                }
                Text {
                    text: "About 20 min"
                    color: Theme.muted
                    font.pixelSize: 13
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Repeater {
                    model: page.steps

                    delegate: AbstractButton {
                        id: stepPill
                        required property var modelData
                        required property int index
                        readonly property bool current: index === page.step
                        readonly property bool reachable: index <= page.reached && page.stepId !== "done"
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        implicitHeight: 34
                        enabled: reachable && !current
                        focusPolicy: Qt.NoFocus
                        onClicked: page.go(index)

                        background: Rectangle {
                            radius: 17
                            color: stepPill.current ? Theme.accent : stepPill.index < page.reached || page.stepId === "done" ? Theme.accentSoft : Theme.sidebar
                        }
                        contentItem: Text {
                            text: (stepPill.index + 1) + ". " + stepPill.modelData.label
                            color: stepPill.current ? Theme.inkOnAccent : stepPill.index <= page.reached ? Theme.accentStrong : Theme.faint
                            font.pixelSize: 13
                            font.weight: stepPill.current ? Font.DemiBold : Font.Normal
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        ColumnLayout {
            id: mainColumn
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Loader {
                Layout.fillWidth: true
                sourceComponent: {
                    switch (page.stepId) {
                    case "grammar": return grammarStep
                    case "dialogue": return dialogueStep
                    case "practice": return practiceStep
                    case "speak": return speakStep
                    case "done": return doneStep
                    default: return vocabStep
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                visible: page.stepId !== "done"
                spacing: 12

                PrimaryButton {
                    secondary: true
                    visible: page.step > 0
                    text: "Back"
                    onClicked: page.go(page.step - 1)
                }
                Item {
                    Layout.fillWidth: true
                }
                PrimaryButton {
                    enabled: page.canNext
                    text: page.step + 1 < page.steps.length ? "Next: " + page.steps[page.step + 1].label : ""
                    onClicked: page.go(page.step + 1)
                }
            }
        }

        // ---- Beside the step: the lesson goals and its words ---------------------------------------------
        ColumnLayout {
            Layout.fillWidth: !page.wide
            Layout.preferredWidth: page.wide ? 340 : -1
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "Lesson goals"
                subtitle: (page.unit.best ?? -1) >= 0 ? "Best so far " + page.unit.best + "%" : ""
                icon: "target"

                Repeater {
                    model: page.goals

                    delegate: AbstractButton {
                        id: goalRow
                        required property var modelData
                        readonly property bool done: page.goalDone(modelData)
                        readonly property bool current: modelData.step === page.step
                        Layout.fillWidth: true
                        implicitHeight: 34
                        enabled: modelData.step <= page.reached && page.stepId !== "done" && !current
                        focusPolicy: Qt.StrongFocus
                        onClicked: page.go(modelData.step)
                        Accessible.name: modelData.label + (done ? ", done" : current ? ", current step" : "")

                        background: Rectangle {
                            radius: 8
                            color: goalRow.hovered && goalRow.enabled ? Theme.hover : "transparent"
                            border.color: goalRow.visualFocus ? Theme.accent : "transparent"
                        }
                        contentItem: RowLayout {
                            spacing: 10

                            Rectangle {
                                implicitWidth: 22
                                implicitHeight: 22
                                radius: 11
                                color: goalRow.done ? Theme.accent : "transparent"
                                border.color: goalRow.done || goalRow.current ? Theme.accent : Theme.edge
                                border.width: 2

                                Icon {
                                    anchors.centerIn: parent
                                    visible: goalRow.done
                                    name: "check"
                                    size: 13
                                    color: Theme.inkOnAccent
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: goalRow.modelData.label
                                color: goalRow.done || goalRow.current ? Theme.ink : Theme.muted
                                font.pixelSize: 14
                                font.weight: goalRow.current ? Font.DemiBold : Font.Normal
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }

            // The lesson's words, to glance back at from a later step; never during practice, where the
            // translations would give the answers away.
            Card {
                Layout.fillWidth: true
                visible: page.wide && page.stepId !== "vocab" && page.stepId !== "practice"
                title: "Words in this lesson"
                icon: "book"

                Repeater {
                    model: page.unit.vocab ?? []

                    delegate: Rectangle {
                        id: wordRow
                        required property var modelData
                        Layout.fillWidth: true
                        implicitHeight: Math.max(42, wordLine.implicitHeight + 14)
                        radius: Theme.smallRadius
                        color: Theme.raised
                        border.color: Theme.line

                        RowLayout {
                            id: wordLine
                            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                            anchors.leftMargin: 12
                            anchors.rightMargin: 10
                            spacing: 8

                            SpanishWord {
                                Layout.maximumWidth: wordLine.width * 0.55
                                spanish: wordRow.modelData.es
                                gender: wordRow.modelData.gender
                                font.pixelSize: 14
                            }
                            SpeakButton {
                                spanish: wordRow.modelData.es
                                diameter: 26
                            }
                            Text {
                                Layout.fillWidth: true
                                text: wordRow.modelData.en
                                textFormat: Text.PlainText
                                color: Theme.muted
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignRight
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }

    // ---- Steps ------------------------------------------------------------------------------------

    component Callout: Rectangle {
        id: callout
        property string icon: "lightbulb"
        property string label
        property string text
        property color tint: Theme.accent
        property color soft: Theme.accentSoft
        Layout.fillWidth: true
        implicitHeight: calloutRow.implicitHeight + 26
        radius: Theme.smallRadius
        color: soft

        RowLayout {
            id: calloutRow
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
            spacing: 12

            Icon {
                Layout.alignment: Qt.AlignTop
                name: callout.icon
                color: callout.tint
            }
            Text {
                Layout.fillWidth: true
                text: (callout.label !== "" ? "<b>" + callout.label + "</b> " : "")
                      + callout.text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
                textFormat: Text.StyledText
                color: Theme.ink
                font.pixelSize: 15
                wrapMode: Text.WordWrap
            }
        }
    }

    Component {
        id: vocabStep

        ColumnLayout {
            spacing: 16

            Callout {
                visible: (page.unit.coach ?? "") !== ""
                label: "Coach's tip for this lesson:"
                text: page.unit.coach ?? ""
            }
            GridLayout {
                Layout.fillWidth: true
                columns: 2
                rowSpacing: 12
                columnSpacing: 12

                Repeater {
                    model: page.unit.vocab ?? []

                    delegate: Rectangle {
                        id: wordCard
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                        implicitHeight: wordColumn.implicitHeight + 28
                        radius: Theme.smallRadius
                        color: Theme.raised
                        border.color: Theme.line

                        ColumnLayout {
                            id: wordColumn
                            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 14 }
                            spacing: 4

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                SpanishWord {
                                    Layout.fillWidth: true
                                    spanish: wordCard.modelData.es
                                    gender: wordCard.modelData.gender
                                }
                                SpeakButton {
                                    spanish: wordCard.modelData.es
                                    diameter: 32
                                }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: wordCard.modelData.en
                                color: Theme.muted
                                font.pixelSize: 14
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                            Text {
                                Layout.fillWidth: true
                                visible: text !== ""
                                text: wordCard.modelData.example
                                color: Theme.ink
                                font.pixelSize: 14
                                font.italic: true
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                        }
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: "Nouns come with their article in colour: blue for el, rose for la. Learn them together."
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
        }
    }

    Component {
        id: grammarStep

        ColumnLayout {
            readonly property var grammar: page.unit.grammar ?? ({})
            id: grammarColumn
            spacing: 16

            Text {
                Layout.fillWidth: true
                text: grammarColumn.grammar.title ?? ""
                color: Theme.ink
                font.pixelSize: 22
                font.weight: Font.Bold
                wrapMode: Text.WordWrap
                textFormat: Text.PlainText
            }
            Callout {
                visible: (grammarColumn.grammar.hack ?? "") !== ""
                icon: "bolt"
                label: "The hack:"
                text: grammarColumn.grammar.hack ?? ""
            }
            Repeater {
                model: grammarColumn.grammar.tables ?? []

                delegate: Rectangle {
                    id: table
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: tableGrid.implicitHeight + 24
                    radius: Theme.smallRadius
                    color: Theme.raised
                    border.color: Theme.line

                    GridLayout {
                        id: tableGrid
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
                        columns: Math.max(1, table.modelData.head.length)
                        rowSpacing: 8
                        columnSpacing: 16

                        Repeater {
                            model: table.modelData.head

                            delegate: Text {
                                required property string modelData
                                Layout.fillWidth: true
                                text: modelData
                                color: Theme.muted
                                font.pixelSize: 12
                                font.weight: Font.Bold
                                font.capitalization: Font.AllUppercase
                                wrapMode: Text.WordWrap
                            }
                        }
                        Repeater {
                            // Rows flattened cell by cell: the grid wraps them into columns.
                            model: [].concat.apply([], table.modelData.rows)

                            delegate: Text {
                                required property string modelData
                                required property int index
                                Layout.fillWidth: true
                                text: modelData
                                color: Theme.ink
                                font.pixelSize: 15
                                font.weight: index % Math.max(1, table.modelData.head.length) === 0 ? Font.DemiBold : Font.Normal
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                        }
                    }
                }
            }
            Repeater {
                model: grammarColumn.grammar.examples ?? []

                delegate: RowLayout {
                    id: example
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 10

                    SpeakButton {
                        spanish: example.modelData.es
                        diameter: 32
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0

                        Text {
                            Layout.fillWidth: true
                            text: example.modelData.es
                            color: Theme.ink
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                        Text {
                            Layout.fillWidth: true
                            text: example.modelData.en
                            color: Theme.muted
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                    }
                }
            }
            Callout {
                visible: (grammarColumn.grammar.tip ?? "") !== ""
                icon: "sparkles"
                label: "Tip:"
                text: grammarColumn.grammar.tip ?? ""
                tint: Theme.purple
                soft: Theme.purpleSoft
            }
        }
    }

    Component {
        id: dialogueStep

        ColumnLayout {
            id: dialogue
            property bool english: false
            spacing: 12

            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    text: "Read it out loud. You are “Tú”. Try it first without the English."
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
                Chip {
                    text: "Show English"
                    checked: dialogue.english
                    onClicked: dialogue.english = !dialogue.english
                }
            }
            Repeater {
                model: page.unit.dialogue ?? []

                delegate: RowLayout {
                    id: line
                    required property var modelData
                    Layout.fillWidth: true
                    layoutDirection: modelData.me ? Qt.RightToLeft : Qt.LeftToRight
                    spacing: 10

                    SpeakButton {
                        Layout.alignment: Qt.AlignTop
                        spanish: line.modelData.es
                        voice: line.modelData.voice
                        diameter: 32
                    }
                    Rectangle {
                        Layout.maximumWidth: mainColumn.width * 0.7
                        Layout.preferredWidth: bubble.implicitWidth + 28
                        implicitHeight: bubble.implicitHeight + 20
                        radius: 14
                        color: line.modelData.me ? Theme.accentSoft : Theme.raised
                        border.color: line.modelData.me ? Theme.accentLine : Theme.line

                        ColumnLayout {
                            id: bubble
                            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 14 }
                            spacing: 2

                            Text {
                                text: line.modelData.speaker
                                color: line.modelData.me ? Theme.accentStrong : Theme.muted
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                            }
                            Text {
                                Layout.fillWidth: true
                                Layout.maximumWidth: mainColumn.width * 0.7 - 28
                                text: line.modelData.es
                                color: Theme.ink
                                font.pixelSize: 16
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                            Text {
                                Layout.fillWidth: true
                                Layout.maximumWidth: mainColumn.width * 0.7 - 28
                                visible: dialogue.english
                                text: line.modelData.en
                                color: Theme.muted
                                font.pixelSize: 14
                                font.italic: true
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }

    Component {
        id: practiceStep

        ColumnLayout {
            spacing: 16

            ExerciseSession {
                Layout.fillWidth: true
                visible: page.session.active && !page.session.finished
                focus: visible
                session: page.session
            }
            SessionSummary {
                Layout.fillWidth: true
                visible: page.practiceResult !== null
                result: page.practiceResult ?? ({})
                message: "Practice done. Next, say a few phrases out loud."
            }
        }
    }

    Component {
        id: speakStep

        ColumnLayout {
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: "Listen, then say each phrase out loud two or three times. Tick it when you have."
                color: Theme.muted
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            Repeater {
                model: page.unit.speak ?? []

                delegate: Rectangle {
                    id: phrase
                    required property string modelData
                    required property int index
                    readonly property bool done: page.said.indexOf(index) >= 0
                    Layout.fillWidth: true
                    implicitHeight: phraseRow.implicitHeight + 20
                    radius: Theme.smallRadius
                    color: done ? Theme.successSoft : Theme.raised
                    border.color: done ? Theme.goodLine : Theme.line

                    RowLayout {
                        id: phraseRow
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 12 }
                        spacing: 10

                        SpeakButton {
                            spanish: phrase.modelData
                            diameter: 34
                        }
                        SpeakButton {
                            spanish: phrase.modelData
                            slow: true
                            diameter: 30
                        }
                        Text {
                            Layout.fillWidth: true
                            text: phrase.modelData
                            color: Theme.ink
                            font.pixelSize: 17
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                        Chip {
                            text: phrase.done ? "Said it" : "I said it"
                            checked: phrase.done
                            onClicked: page.said = phrase.done ? page.said.filter(i => i !== phrase.index)
                                                               : page.said.concat([phrase.index])
                        }
                    }
                }
            }
        }
    }

    Component {
        id: doneStep

        ColumnLayout {
            spacing: 16

            SessionSummary {
                Layout.fillWidth: true
                visible: page.practiceResult !== null
                result: page.practiceResult ?? ({})
                title: "Lesson " + (page.unit.n ?? "") + " complete!"
                message: page.saved === null ? ""
                         : (page.saved.newCards > 0 ? page.saved.newCards + " new words joined your review deck. " : "")
                           + (page.said.length > 0 ? page.said.length + " phrases said out loud." : "")
                nextStep: page.nextStep
                showReflection: lessonReflection.wanted
                onFollow: route => page.openRoute(route)

                reflection: ReflectionCard {
                    id: lessonReflection
                    Layout.fillWidth: true
                    learner: page.learner
                    asked: page.practiceResult?.reflect ?? false
                    activity: "lesson"
                    kinds: page.practiceResult?.kinds ?? []
                }

                PrimaryButton {
                    secondary: true
                    text: "Back to Today"
                    onClicked: page.openRoute("today")
                }
                Item {
                    Layout.fillWidth: true
                }
                PrimaryButton {
                    visible: (page.unit.next ?? -1) >= 0
                    text: "Next lesson"
                    onClicked: page.openRoute("lesson/" + page.unit.next)
                }
            }
            Callout {
                visible: page.practiceResult === null
                icon: "target"
                text: "Finish the practice step first: the lesson is saved with its score."
            }
            PrimaryButton {
                visible: page.practiceResult === null
                text: "Go to practice"
                onClicked: page.go(3)
            }
        }
    }
}
