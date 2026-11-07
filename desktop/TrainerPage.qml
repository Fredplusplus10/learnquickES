import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A focus trainer from the daily plan (verbs, past tenses, cognates, false friends, gender, speaking,
// describing, mixed) or the spaced-repetition review. Finishing a round ticks the plan's task.
// On a wide page the learner's record in the trainer's skill and the cheat sheet (folded) sit beside the round.
ScrollPage {
    id: page

    required property LearnerController learner
    required property SessionModel session
    property string trainerId: "verbs"

    readonly property var info: learner.trainer(trainerId)
    property var result: null
    property var nextStep: ({})
    property bool showReference: trainerId === "speak"

    maxWidth: 1240
    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 1000

    // Where the trainer sits on the Practice screen, and the skill its answers count towards (as on Practice).
    readonly property string groupName: {
        switch (trainerId) {
        case "verbs": case "past": case "gender": return "Grammar"
        case "cognates": case "falsefriends": return "Words"
        case "speak": case "describe": return "Speaking and describing"
        default: return "Every day"
        }
    }
    readonly property string skillId: {
        switch (trainerId) {
        case "review": return "review"
        case "mixed": return "grammar"
        case "verbs": case "past": case "gender": case "cognates": case "falsefriends": return trainerId
        default: return ""
        }
    }
    property var report: learner.coachReport()
    readonly property var skill: skillId === "" ? null : (report.skills ?? []).find(s => s.id === skillId) ?? null

    // The cheat sheet, as rows of a title and a detail.
    readonly property var reference: {
        switch (trainerId) {
        case "verbs": return (info.tenses ?? []).map(t => ({ title: t.en, detail: t.cue }))
        case "past": return (info.reasons ?? []).map(r => ({ title: r, detail: "" }))
        case "cognates": return (info.rules ?? []).map(r => ({ title: r.rule, detail: r.examples }))
        case "falsefriends":
            return (info.falseFriends ?? []).map(f => ({
                title: f.es + " = " + f.meaning,
                detail: "Not “" + f.looksLike + "”: that is " + f.realWord
            }))
        case "gender": return (info.rules ?? []).map(r => ({ title: r.rule, detail: r.examples + (r.note ? " · " + r.note : "") }))
        case "speak": return info.group ? [{ title: info.group.title, detail: info.group.tip }] : []
        case "describe": return (info.frames ?? []).map(f => ({ title: f, detail: "" }))
        default: return []
        }
    }

    function start() {
        result = null
        nextStep = {}
        session.startTrainer(trainerId)
        scrollToTop()
    }

    Component.onCompleted: start()

    Connections {
        target: page.session
        function onCompleted() {
            page.result = page.session.result
            page.nextStep = page.learner.nextStep(page.trainerId)
            page.report = page.learner.coachReport()
        }
    }

    Keys.onReturnPressed: if (result !== null) page.openRoute("today")
    Keys.onEnterPressed: if (result !== null) page.openRoute("today")

    Breadcrumb {
        backLabel: "Practice"
        trail: page.groupName
        onBack: page.openRoute("practice")
    }

    PageHero {
        overline: page.trainerId === "review" ? "Spaced repetition" : "Trainer"
        title: page.info.title ?? ""
        subtitle: page.info.description ?? ""
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        // The learner's record and the cheat sheet. On a narrow page only the sheet shows, above the round.
        ColumnLayout {
            Layout.fillWidth: !page.wide
            Layout.preferredWidth: page.wide ? 340 : -1
            Layout.maximumWidth: page.wide ? 340 : -1
            Layout.alignment: Qt.AlignTop
            Layout.row: 0
            Layout.column: page.wide ? 1 : 0
            visible: referenceCard.visible || recordCard.visible
            spacing: 20

            Card {
                id: recordCard
                visible: page.wide
                Layout.fillWidth: true
                title: "Your record"
                subtitle: "Last " + (page.report.windowDays ?? 14) + " days"
                icon: "chart"

                RowLayout {
                    Layout.fillWidth: true
                    visible: page.skill !== null && page.skill.enough
                    spacing: 10

                    Text {
                        Layout.fillWidth: true
                        text: (page.skill?.label ?? "") + ", right first try\n" + (page.skill?.correct ?? 0) + " of "
                              + (page.skill?.total ?? 0) + " answers"
                        color: Theme.muted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        text: (page.skill?.accuracy ?? 0) + "%"
                        color: (page.skill?.accuracy ?? 0) >= (page.report.weakBelow ?? 70) ? Theme.goodInk : Theme.accentStrong
                        font.pixelSize: 22
                        font.weight: Font.Bold
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.skill === null || !page.skill.enough
                    text: page.skillId === "" ? "You check these yourself, so there is no score: saying it out loud is what counts."
                          : (page.skill?.total ?? 0) === 0 ? "No answers yet. This round starts your record."
                          : (page.skill?.total ?? 0) + ((page.skill?.total ?? 0) === 1 ? " answer" : " answers")
                            + " so far. Your coach judges a skill from " + (page.report.minAnswers ?? 8) + "."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.skillId !== ""
                    text: "Misses come back in a second try at the end of the round, worth a little less than a first try."
                    color: Theme.faint
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Card {
                id: referenceCard
                visible: page.reference.length > 0
                Layout.fillWidth: true

                AbstractButton {
                    id: referenceToggle
                    Layout.fillWidth: true
                    focusPolicy: Qt.StrongFocus
                    onClicked: page.showReference = !page.showReference
                    Keys.onReturnPressed: clicked()
                    Keys.onEnterPressed: clicked()
                    Accessible.name: (page.trainerId === "speak" ? "Today's sound" : "Cheat sheet")
                                     + (page.showReference ? ", shown" : ", hidden")

                    contentItem: RowLayout {
                        spacing: 10

                        Icon {
                            name: "lightbulb"
                            color: Theme.accent
                            size: 20
                        }
                        Text {
                            Layout.fillWidth: true
                            text: page.trainerId === "speak" ? "Today's sound" : "Cheat sheet"
                            color: Theme.ink
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: page.showReference ? "Hide" : "Show"
                            color: referenceToggle.hovered || referenceToggle.visualFocus ? Theme.accentStrong : Theme.muted
                            font.pixelSize: 13
                        }
                    }
                    background: null
                }
                Repeater {
                    // Folded until asked for: open, it would give the answers away (only today's sound starts open).
                    model: page.showReference ? page.reference : []

                    delegate: ColumnLayout {
                        id: row
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        spacing: 1

                        Rectangle {
                            visible: row.index > 0
                            Layout.fillWidth: true
                            Layout.bottomMargin: 8
                            implicitHeight: 1
                            color: Theme.line
                        }
                        Text {
                            Layout.fillWidth: true
                            text: row.modelData.title
                            color: Theme.ink
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: text !== ""
                            text: row.modelData.detail
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                    }
                }
            }
        }

        // The round, then its summary.
        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            Layout.row: page.wide ? 0 : 1
            Layout.column: 0
            spacing: 20

            Card {
                Layout.fillWidth: true
                visible: page.session.active && !page.session.finished

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
                nextStep: page.nextStep
                showReflection: trainerReflection.wanted
                onFollow: route => page.openRoute(route)
                title: (page.result?.empty ?? false) ? (page.trainerId === "review" ? "Nothing to review" : "Nothing here yet")
                                                     : summaryTitle()
                illustration: !(page.result?.empty ?? false) ? ""
                              : page.trainerId === "review" && (page.info.nextDue ?? "") !== "" ? "empty-caught-up"
                              : "empty-nothing-yet"
                message: {
                    if (page.result === null)
                        return ""
                    if (page.result.empty) {
                        if (page.trainerId === "review")
                            return (page.info.nextDue ?? "") !== "" ? "All caught up. The next card is due " + page.info.nextDue + "."
                                                                    : "Your deck is empty: words join it when you finish a lesson."
                        return "Finish your first lesson to unlock this practice."
                    }
                    if (page.result.answered === 0)
                        return page.result.selfRated + (page.result.selfRated === 1 ? " item done" : " items done")
                               + (page.result.marked ? " · task ticked for today." : ".")
                    return page.result.firstTry + " of " + page.result.answered + " right on the first try"
                           + (page.result.marked ? " · task ticked for today." : ".")
                }

                reflection: ReflectionCard {
                    id: trainerReflection
                    Layout.fillWidth: true
                    learner: page.learner
                    asked: page.result?.reflect ?? false
                    activity: page.trainerId
                    kinds: page.result?.kinds ?? []
                }

                function summaryTitle() {
                    const p = page.result?.percent ?? 0
                    return (page.result?.answered ?? 0) === 0 ? "¡Bien hecho!" : p >= 90 ? "¡Excelente!" : p >= 70 ? "¡Muy bien!" : "¡Sigue así!"
                }

                PrimaryButton {
                    text: "Back to Today"
                    onClicked: page.openRoute("today")
                }
                PrimaryButton {
                    secondary: true
                    visible: page.trainerId !== "review" && !(page.result?.empty ?? false)
                    text: "Another round"
                    onClicked: page.start()
                }
                PrimaryButton {
                    secondary: true
                    text: "All trainers"
                    onClicked: page.openRoute("practice")
                }
            }
        }
    }
}
