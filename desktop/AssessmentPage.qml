import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A level assessment (A1, A2 or B1): the test behind each Certificate of Readiness. Reading, listening, and
// grammar and vocabulary, all marked by the app, with no self-assessment. No answer is marked while it runs,
// and the result comes at the end. It passes with 70% overall and 60% in every part; passing issues the
// certificate. Leaving before the end does not count, and the questions stay the same until the next day.
ScrollPage {
    id: page

    required property LearnerController learner
    required property SessionModel session
    property string level: "A1"

    maxWidth: 1240
    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 1000

    readonly property var entry: {
        for (const e of Certificates.levels) {
            if (e.level === page.level)
                return e
        }
        return null
    }
    readonly property var steps: [
        { id: "intro", label: "Start" },
        { id: "reading", label: "Reading" },
        { id: "listening", label: "Listening" },
        { id: "language", label: "Grammar & words" },
        { id: "result", label: "Result" }
    ]
    property int step: 0
    readonly property string stepId: steps[step].id
    readonly property bool part: ["reading", "listening", "language"].indexOf(stepId) >= 0
    readonly property bool running: session.active && !session.finished

    property var draw: ({})
    property var tallies: ({})
    property var result: null

    function go(index) {
        Speech.stop()
        session.stop()
        step = index
        scrollToTop()
        if (part)
            session.startAssessmentPart(stepId)
        if (stepId === "result" && result === null) {
            result = Certificates.finishAssessment(page.level, {
                reading: tallies.reading ?? [0, 0],
                listening: tallies.listening ?? [0, 0],
                language: tallies.language ?? [0, 0]
            })
        }
    }

    function begin() {
        draw = session.startAssessment(page.level, Certificates.assessmentSeed(page.level))
        if (draw.level === undefined)
            return
        tallies = {}
        result = null
        go(1)
    }

    function partStatus(id) {
        const index = steps.findIndex(s => s.id === id)
        if (tallies[id] !== undefined)
            return "Done"
        if (index === step)
            return "Now"
        return (draw[id] ?? 0) > 0 ? draw[id] + " questions" : "To do"
    }

    Connections {
        target: page.session
        function onCompleted() {
            const r = page.session.result
            if (page.session.kind !== "assessment")
                return
            const next = Object.assign({}, page.tallies)
            next[r.part] = [r.firstTry, r.answered]
            page.tallies = next
            // On to the next part, or to the result, once the session has finished emitting.
            const following = page.step + 1
            Qt.callLater(() => page.go(following))
        }
    }

    Breadcrumb {
        backLabel: "Certificates"
        trail: page.level + " assessment"
        onBack: page.openRoute("certificates")
    }

    PageHero {
        overline: "Prueba de nivel " + page.level
        title: page.level + " level assessment"
        subtitle: "Reading, listening, and grammar and vocabulary, all marked by LearnQuick. About 20 minutes."
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
                    case "language": return languageStep
                    default: return resultStep
                    }
                }
            }
        }

        ColumnLayout {
            Layout.preferredWidth: 340
            Layout.maximumWidth: 340
            Layout.alignment: Qt.AlignTop
            visible: page.wide
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "Your assessment"
                subtitle: page.stepId === "result" ? "Finished" : page.step === 0 ? "Not started" : "Part " + page.step + " of 3"
                icon: "award"

                Repeater {
                    model: page.steps.slice(1, 4)

                    delegate: RowLayout {
                        id: partRow
                        required property var modelData
                        required property int index
                        readonly property bool current: index + 1 === page.step
                        readonly property bool done: page.tallies[modelData.id] !== undefined
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
                                text: partRow.index + 1
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
                title: "How it is marked"
                icon: "target"

                Text {
                    Layout.fillWidth: true
                    text: "It passes with 70% of all the questions and at least 60% in each part. Your marks "
                          + "appear at the end, not after each answer."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "Speaking and writing are not part of it: practise them with the B1 mock exam and a "
                          + "language exchange."
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
            title: page.entry && page.entry.earned ? "You already have this certificate"
                                                   : page.entry && page.entry.assessmentOpen ? "Three parts" : "Not open yet"

            Repeater {
                model: page.entry && page.entry.assessmentOpen ? [
                    { name: "Reading", detail: "a short text, 5 questions" },
                    { name: "Listening", detail: "a message read aloud, 5 questions" },
                    { name: "Grammar and vocabulary", detail: "12 questions" }
                ] : []

                delegate: RowLayout {
                    id: partLine
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
                        text: "<b>" + partLine.modelData.name + "</b> · " + partLine.modelData.detail
                        textFormat: Text.StyledText
                        color: Theme.ink
                        font.pixelSize: 15
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: page.entry === null ? ""
                      : page.entry.earned ? "Issued to " + page.entry.username + " on " + page.entry.issuedOn + "."
                      : page.entry.assessmentOpen
                        ? "Take it in one sitting: leaving before the end does not count, and today's questions stay "
                          + "the same until tomorrow. You can take it once a day. " + page.entry.lockedReason
                        : page.entry.lockedReason
                textFormat: Text.PlainText
                color: Theme.muted
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            RowLayout {
                spacing: 12

                PrimaryButton {
                    visible: page.entry !== null && page.entry.assessmentOpen
                    text: "Start the assessment"
                    onClicked: page.begin()
                }
                PrimaryButton {
                    secondary: page.entry !== null && page.entry.assessmentOpen
                    text: "Back to certificates"
                    onClicked: page.openRoute(page.entry && page.entry.earned ? "certificates/" + page.level : "certificates")
                }
            }
        }
    }

    Component {
        id: readingStep

        ColumnLayout {
            spacing: 16

            Passage {
                text: page.draw.readingText ?? ""
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
                        spanish: page.draw.listeningText ?? ""
                        diameter: 56
                    }
                    SpeakButton {
                        spanish: page.draw.listeningText ?? ""
                        slow: true
                        diameter: 42
                    }
                    Text {
                        Layout.fillWidth: true
                        text: (page.draw.listeningIntro ?? "") + " Listen twice, then answer."
                        color: Theme.muted
                        font.pixelSize: 15
                        wrapMode: Text.WordWrap
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: !Speech.available
                    text: "This computer has no Spanish voice, so read the message instead:"
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
            }
            Passage {
                visible: !Speech.available
                text: page.draw.listeningText ?? ""
            }
            PartSession {}
        }
    }

    Component {
        id: languageStep

        PartSession {}
    }

    Component {
        id: resultStep

        ColumnLayout {
            id: outcome
            readonly property var r: page.result ?? ({})
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
                            Layout.fillWidth: true
                            text: outcome.r.pass ? "¡Aprobado! " + page.level + " assessment passed" : "Not yet: keep going"
                            color: Theme.ink
                            font.pixelSize: 24
                            font.weight: Font.Bold
                            wrapMode: Text.WordWrap
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "Overall " + (outcome.r.overall ?? 0) + "%. It passes with 70% overall and 60% in each part."
                                  + (outcome.r.pass ? "" : outcome.r.overallPassed ? " One part is below 60%." : "")
                            color: Theme.muted
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                        }
                    }
                }
                Repeater {
                    model: [
                        { label: "Reading", value: outcome.r.reading ?? 0 },
                        { label: "Listening", value: outcome.r.listening ?? 0 },
                        { label: "Grammar and vocabulary", value: outcome.r.language ?? 0 }
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
                            Accessible.role: Accessible.ProgressBar
                            Accessible.name: bar.modelData.label + " " + bar.modelData.value + "%"

                            Rectangle {
                                width: parent.width * bar.modelData.value / 100
                                height: parent.height
                                radius: 4
                                color: bar.modelData.value >= 60 ? Theme.success : Theme.accent
                            }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: outcome.r.issued ? "Your " + page.level + " Certificate of Readiness is ready."
                          : outcome.r.pass ? ""
                          : "Review the lessons of the level, then try again tomorrow."
                    visible: text !== ""
                    color: Theme.ink
                    font.pixelSize: 15
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    visible: (outcome.r.error ?? "") !== ""
                    text: outcome.r.error ?? ""
                    textFormat: Text.PlainText
                    color: Theme.danger
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
            }
            RowLayout {
                spacing: 12

                PrimaryButton {
                    text: outcome.r.issued ? "See your certificate" : "Back to certificates"
                    onClicked: page.openRoute(outcome.r.issued ? "certificates/" + page.level : "certificates")
                }
                PrimaryButton {
                    secondary: true
                    visible: !outcome.r.pass
                    text: "Go to Lessons"
                    onClicked: page.openRoute("lessons")
                }
            }
        }
    }
}
