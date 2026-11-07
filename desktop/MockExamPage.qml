import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The B1 mock exam, shaped like the DELE B1: reading, listening and grammar are scored automatically;
// writing and speaking are partly self-assessed. Passing needs 60% in the written and the oral group.
// On a wide page, how far the learner has got and the score of each part so far sit beside the step.
ScrollPage {
    id: page

    required property LearnerController learner
    required property SessionModel session
    property string initialStep: "intro"

    readonly property var mock: learner.mockExam()
    readonly property var steps: [
        { id: "intro", label: "Start" },
        { id: "reading", label: "Reading" },
        { id: "listening", label: "Listening" },
        { id: "grammar", label: "Grammar" },
        { id: "writing", label: "Writing" },
        { id: "speaking", label: "Speaking" },
        { id: "result", label: "Result" }
    ]
    property int step: Math.max(0, steps.findIndex(s => s.id === initialStep))
    readonly property string stepId: steps[step].id
    readonly property bool autoPart: ["reading", "listening", "grammar"].indexOf(stepId) >= 0
    readonly property bool running: session.active && !session.finished

    // Results so far: [correct, total] per automatic part, writing points, one rating per speaking task.
    property var tallies: ({})
    property string essay
    property var selfChecks: []
    property var ratings: []
    property var result: null

    readonly property var checks: learner.writingChecks(essay)
    readonly property int writingPoints: checks.filter(c => c.ok).length + selfChecks.length
    readonly property int wordCount: essay.trim() === "" ? 0 : essay.trim().split(/\s+/).length
    readonly property bool partDone: !autoPart || tallies[stepId] !== undefined
    // filter() skips the holes of a sparse array, so this counts the tasks actually rated.
    readonly property bool allRated: ratings.filter(r => r !== undefined).length === (mock.speaking ?? []).length

    maxWidth: 1240
    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 1000

    // Where each part stands, for the side panel.
    function partStatus(id) {
        const index = steps.findIndex(s => s.id === id)
        if (tallies[id] !== undefined)
            return tallies[id][0] + " of " + tallies[id][1]
        if (id === "writing" && step > index)
            return wordCount > 0 ? wordCount + " words" : "Skipped"
        if (id === "writing" && step === index)
            return wordCount > 0 ? wordCount + " words so far" : "Now"
        if (id === "speaking" && step >= index) {
            const rated = ratings.filter(r => r !== undefined).length
            return rated + " of " + (mock.speaking ?? []).length + " rated"
        }
        return index === step ? "Now" : index < step ? "Skipped" : "To do"
    }

    function go(index) {
        Speech.stop()
        session.stop()
        step = index
        scrollToTop()
        enterStep()
    }

    function restart() {
        tallies = {}
        essay = ""
        selfChecks = []
        ratings = []
        result = null
        go(0)
    }

    function enterStep() {
        if (autoPart && tallies[stepId] === undefined)
            session.startMockPart(stepId)
        if (stepId === "result" && result === null) {
            result = learner.finishMock({
                reading: tallies.reading ?? [0, 0],
                listening: tallies.listening ?? [0, 0],
                grammar: tallies.grammar ?? [0, 0],
                writing: writingPoints,
                speaking: ratings.map(r => r ?? 0)
            })
        }
    }

    Component.onCompleted: enterStep()

    Connections {
        target: page.session
        function onCompleted() {
            const r = page.session.result
            const next = Object.assign({}, page.tallies)
            next[r.part] = [r.firstTry, r.answered]
            page.tallies = next
        }
    }

    Breadcrumb {
        backLabel: "Lessons"
        trail: "Final exam"
        onBack: page.openRoute("lessons")
    }

    PageHero {
        overline: "Simulacro DELE B1"
        title: "B1 mock exam"
        subtitle: (page.mock.best ?? -1) >= 0 ? "Your best so far: " + page.mock.best + "%. Five parts shaped like the DELE B1."
                                              : "Five parts shaped like the DELE B1, about 60 minutes. Take it in one sitting if you can."
    }

    // The steps as pills; on a wide page the side panel lists them instead.
    RowLayout {
        Layout.fillWidth: true
        visible: !page.wide
        spacing: 6

        Repeater {
            model: page.steps

            delegate: Rectangle {
                id: pill
                required property var modelData
                required property int index
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                implicitHeight: 30
                radius: 15
                color: index === page.step ? Theme.accent : index < page.step ? Theme.accentSoft : Theme.sidebar

                Text {
                    anchors.centerIn: parent
                    text: pill.modelData.label
                    color: pill.index === page.step ? Theme.inkOnAccent : pill.index < page.step ? Theme.accentStrong : Theme.faint
                    font.pixelSize: 12
                    font.weight: pill.index === page.step ? Font.DemiBold : Font.Normal
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
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Loader {
                Layout.fillWidth: true
                sourceComponent: {
                    switch (page.stepId) {
                    case "intro": return introStep
                    case "reading": return readingStep
                    case "listening": return listeningStep
                    case "grammar": return grammarStep
                    case "writing": return writingStep
                    case "speaking": return speakingStep
                    default: return resultStep
                    }
                }
            }

            // A finished automatic part: its score, then on to the next.
            Card {
                Layout.fillWidth: true
                visible: page.autoPart && page.partDone

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 14

                    Icon {
                        name: "check"
                        color: Theme.success
                        size: 26
                    }
                    Text {
                        Layout.fillWidth: true
                        text: page.partDone && page.autoPart
                              ? page.steps[page.step].label + ": " + page.tallies[page.stepId][0] + " of " + page.tallies[page.stepId][1] + " right."
                              : ""
                        color: Theme.ink
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                    }
                    PrimaryButton {
                        text: page.step + 1 < page.steps.length ? "Next: " + page.steps[page.step + 1].label : ""
                        onClicked: page.go(page.step + 1)
                    }
                }
            }
        }

        // The exam so far, part by part.
        ColumnLayout {
            Layout.preferredWidth: 340
            Layout.maximumWidth: 340
            Layout.alignment: Qt.AlignTop
            visible: page.wide
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "Your exam"
                subtitle: page.stepId === "result" ? "Finished" : "Part " + Math.max(1, Math.min(page.step, 5)) + " of 5"
                icon: "award"

                Repeater {
                    model: page.steps.slice(1, 6)

                    delegate: RowLayout {
                        id: partRow
                        required property var modelData
                        required property int index
                        readonly property int stepIndex: index + 1
                        readonly property bool current: stepIndex === page.step
                        readonly property bool skipped: stepIndex < page.step && page.partStatus(modelData.id) === "Skipped"
                        readonly property bool done: stepIndex < page.step && !skipped
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            implicitWidth: 24
                            implicitHeight: 24
                            radius: 12
                            color: partRow.current ? Theme.accent : partRow.done ? Theme.success : Theme.background
                            border.color: partRow.current ? Theme.accent : partRow.done ? Theme.success : Theme.line

                            Icon {
                                anchors.centerIn: parent
                                visible: partRow.done
                                name: "check"
                                size: 13
                                color: Theme.inkOnAccent
                            }
                            Text {
                                anchors.centerIn: parent
                                visible: !partRow.done
                                text: partRow.skipped ? "–" : partRow.stepIndex
                                color: partRow.current ? Theme.inkOnAccent : Theme.muted
                                font.pixelSize: 11
                                font.weight: Font.DemiBold
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            text: partRow.modelData.label
                            color: Theme.ink
                            font.pixelSize: 14
                            font.weight: partRow.current ? Font.DemiBold : Font.Normal
                        }
                        Text {
                            text: page.partStatus(partRow.modelData.id)
                            color: partRow.current ? Theme.accentStrong : Theme.muted
                            font.pixelSize: 13
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                title: "How it is scored"
                icon: "target"

                Text {
                    Layout.fillWidth: true
                    text: "Like the real DELE B1, it passes at 60 out of 100 with at least 30 in each group: written "
                          + "(reading and writing) and oral (listening and speaking)."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "Reading, listening and grammar are marked for you. Writing and speaking are partly "
                          + "self-assessed: be honest with yourself."
                    color: Theme.faint
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    component Passage: Rectangle {
        id: passage
        property string text
        Layout.fillWidth: true
        implicitHeight: passageText.implicitHeight + 36
        radius: Theme.smallRadius
        color: Theme.raised
        border.color: Theme.line

        Text {
            id: passageText
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 18 }
            text: passage.text
            color: Theme.ink
            font.pixelSize: 16
            lineHeight: 1.3
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
        }
    }

    component PartSession: Card {
        Layout.fillWidth: true
        visible: page.running

        ExerciseSession {
            Layout.fillWidth: true
            focus: visible
            session: page.session
        }
    }

    Component {
        id: introStep

        Card {
            title: "Five parts"

            Repeater {
                model: [
                    { name: "Reading", detail: "an email, 5 questions" },
                    { name: "Listening", detail: "a voicemail, 5 questions" },
                    { name: "Grammar", detail: "10 questions" },
                    { name: "Writing", detail: "an email of 100–150 words" },
                    { name: "Speaking", detail: "4 timed tasks, out loud" }
                ]

                delegate: RowLayout {
                    id: part
                    required property var modelData
                    Layout.fillWidth: true
                    spacing: 10

                    Rectangle {
                        implicitWidth: 8
                        implicitHeight: 8
                        radius: 4
                        color: Theme.accent
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "<b>" + part.modelData.name + "</b> · " + part.modelData.detail
                        textFormat: Text.StyledText
                        color: Theme.ink
                        font.pixelSize: 15
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: "The real DELE B1 passes at 60/100 with at least 30 in each group (written: reading + writing; "
                      + "oral: listening + speaking). Writing and speaking here are partly self-assessed: be honest with yourself."
                color: Theme.muted
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            PrimaryButton {
                text: "Start the exam"
                onClicked: page.go(1)
            }
        }
    }

    Component {
        id: readingStep

        ColumnLayout {
            spacing: 16

            Passage {
                text: page.mock.readingText ?? ""
            }
            PartSession {}
        }
    }

    Component {
        id: listeningStep

        ColumnLayout {
            spacing: 16

            Card {
                Layout.fillWidth: true

                RowLayout {
                    visible: Speech.available
                    spacing: 12

                    SpeakButton {
                        spanish: page.mock.listeningText ?? ""
                        diameter: 56
                    }
                    SpeakButton {
                        spanish: page.mock.listeningText ?? ""
                        slow: true
                        diameter: 42
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "A voicemail from a friend. Listen twice, then answer."
                        color: Theme.muted
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: !Speech.available
                    text: "This computer has no Spanish voice, so read the voicemail instead:"
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
            }
            Passage {
                visible: !Speech.available || page.partDone
                text: page.mock.listeningText ?? ""
            }
            PartSession {}
        }
    }

    Component {
        id: grammarStep

        PartSession {}
    }

    Component {
        id: writingStep

        ColumnLayout {
            spacing: 16

            Passage {
                text: page.mock.writingPrompt ?? ""
            }
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 240
                radius: Theme.smallRadius
                color: Theme.raised
                border.color: essayArea.activeFocus ? Theme.accent : Theme.line
                border.width: essayArea.activeFocus ? 2 : 1

                ScrollView {
                    anchors.fill: parent
                    anchors.margins: 4

                    TextArea {
                        id: essayArea
                        onTextChanged: page.essay = text
                        placeholderText: "Querido/a…"
                        wrapMode: TextArea.Wrap
                        font.pixelSize: 16
                        color: Theme.ink
                        selectByMouse: true
                        background: null
                        Component.onCompleted: {
                            text = page.essay
                            forceActiveFocus()
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true

                SpanishKeyboard {
                    Layout.fillWidth: true
                    Layout.maximumWidth: implicitWidth
                    Layout.alignment: Qt.AlignTop
                    target: essayArea
                    multiline: true
                }
                Item {
                    Layout.fillWidth: true
                }
                Text {
                    Layout.alignment: Qt.AlignTop
                    text: page.wordCount + " words"
                    color: page.wordCount >= 100 && page.wordCount <= 150 ? Theme.success : Theme.muted
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
            }
            Card {
                Layout.fillWidth: true
                title: "Automatic checks"
                subtitle: page.checks.filter(c => c.ok).length + " of " + page.checks.length

                Repeater {
                    model: page.checks

                    delegate: RowLayout {
                        id: check
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 10

                        Icon {
                            name: check.modelData.ok ? "check" : "x"
                            color: check.modelData.ok ? Theme.success : Theme.faint
                            size: 18
                        }
                        Text {
                            Layout.fillWidth: true
                            text: check.modelData.label
                            color: check.modelData.ok ? Theme.ink : Theme.muted
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
            Card {
                Layout.fillWidth: true
                title: "Check it yourself"

                Repeater {
                    model: page.mock.selfChecks ?? []

                    delegate: CheckBox {
                        required property string modelData
                        required property int index
                        text: modelData
                        checked: page.selfChecks.indexOf(index) >= 0
                        font.pixelSize: 14
                        onToggled: page.selfChecks = checked ? page.selfChecks.concat([index])
                                                             : page.selfChecks.filter(i => i !== index)
                    }
                }
            }
            PrimaryButton {
                text: "Continue to speaking"
                onClicked: page.go(page.step + 1)
            }
        }
    }

    Component {
        id: speakingStep

        ColumnLayout {
            spacing: 16

            Text {
                Layout.fillWidth: true
                text: "Four tasks, out loud, about two minutes each. Then rate yourself honestly."
                color: Theme.muted
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            Repeater {
                model: page.mock.speaking ?? []

                delegate: Card {
                    id: task
                    required property var modelData
                    required property int index
                    property int remaining: 120
                    Layout.fillWidth: true
                    title: (index + 1) + ". " + modelData.title
                    subtitle: timer.running ? Math.floor(remaining / 60) + ":" + (remaining % 60 < 10 ? "0" : "") + remaining % 60 : ""

                    Timer {
                        id: timer
                        interval: 1000
                        repeat: true
                        onTriggered: {
                            task.remaining -= 1
                            if (task.remaining <= 0)
                                stop()
                        }
                    }

                    Text {
                        Layout.fillWidth: true
                        text: task.modelData.prompt
                        color: Theme.ink
                        font.pixelSize: 16
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                    PrimaryButton {
                        secondary: true
                        text: timer.running ? "Stop timer" : task.remaining < 120 ? "Restart 2:00 timer" : "Start 2:00 timer"
                        onClicked: {
                            if (timer.running) {
                                timer.stop()
                            } else {
                                task.remaining = 120
                                timer.start()
                            }
                        }
                    }
                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            model: page.mock.rubric ?? []

                            delegate: Chip {
                                required property string modelData
                                required property int index
                                text: index + " · " + modelData
                                checked: page.ratings[task.index] === index
                                onClicked: {
                                    const next = page.ratings.slice()
                                    next[task.index] = index
                                    page.ratings = next
                                }
                            }
                        }
                    }
                }
            }
            PrimaryButton {
                enabled: page.allRated
                text: "See my result"
                onClicked: page.go(page.step + 1)
            }
        }
    }

    Component {
        id: resultStep

        ColumnLayout {
            readonly property var r: page.result ?? ({})
            id: outcome
            spacing: 16

            Card {
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 16

                    Icon {
                        name: outcome.r.pass ? "award" : "target"
                        color: outcome.r.pass ? Theme.success : Theme.accent
                        size: 44
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: outcome.r.pass ? "¡Aprobado! B1 mock passed" : "Not yet: keep going"
                            color: Theme.ink
                            font.pixelSize: 24
                            font.weight: Font.Bold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "Overall " + (outcome.r.overall ?? 0) + "%. Written group " + (outcome.r.written ?? 0)
                                  + "%, oral group " + (outcome.r.oral ?? 0) + "%: each needs 60%."
                            color: Theme.muted
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                        }
                    }
                }
                Repeater {
                    model: [
                        { label: "Reading + grammar", value: outcome.r.reading ?? 0 },
                        { label: "Listening", value: outcome.r.listening ?? 0 },
                        { label: "Writing (checks + self-review)", value: outcome.r.writing ?? 0 },
                        { label: "Speaking (self-rated)", value: outcome.r.speaking ?? 0 }
                    ]

                    delegate: ColumnLayout {
                        id: bar
                        required property var modelData
                        Layout.fillWidth: true
                        spacing: 4

                        RowLayout {
                            Layout.fillWidth: true

                            Text {
                                Layout.fillWidth: true
                                text: bar.modelData.label
                                color: Theme.ink
                                font.pixelSize: 14
                            }
                            Text {
                                text: bar.modelData.value + "%"
                                color: Theme.ink
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                            }
                        }
                        Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 8
                            radius: 4
                            color: Theme.line

                            Rectangle {
                                width: parent.width * bar.modelData.value / 100
                                height: parent.height
                                radius: 4
                                color: bar.modelData.value >= 60 ? Theme.success : Theme.accent
                            }
                        }
                    }
                }
            }
            RowLayout {
                spacing: 12

                PrimaryButton {
                    text: "Back to Today"
                    onClicked: page.openRoute("today")
                }
                PrimaryButton {
                    secondary: true
                    text: "Take it again"
                    onClicked: page.restart()
                }
            }
        }
    }
}
