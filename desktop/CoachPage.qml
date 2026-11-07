import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The coach: every suggestion with its reason and a way to act on it, a short focus for today, the numbers the
// suggestions rest on, what it has learned the learner enjoys, and a way to tell it more. The suggestions come
// from the deterministic rules in lq::coachTips; nothing here is generated.
ScrollPage {
    id: page

    required property LearnerController learner
    maxWidth: 1240

    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    readonly property bool wide: columnWidth >= 960
    readonly property var tips: learner.coachTips
    // Not `data`: every Item already has a property of that name.
    property var report: learner.coachReport()
    readonly property var answeredSkills: (report.skills ?? []).filter(skill => skill.total > 0)

    Connections {
        target: page.learner
        function onChanged() { page.report = page.learner.coachReport() }
    }

    // The icon for each of the coach's rules (lq::CoachTip::id).
    function tipIcon(id) {
        switch (id) {
        case "review": return "cards"
        case "behind": case "plan": return "calendar"
        case "checkpoint": case "skill": return "target"
        case "lesson": return "book"
        case "speak": return "mic"
        case "hard": return "turtle"
        case "easy": return "bolt"
        case "enjoy": return "sparkles"
        case "ahead": return "flame"
        }
        return "lightbulb"
    }
    // What the action does, from where it leads.
    function actionLabel(route) {
        if (route === "practice/review")
            return "Review now"
        if (route.startsWith("lesson/"))
            return "Open lesson"
        if (route.startsWith("practice"))
            return "Open practice"
        return "Open"
    }

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

    // A label and its value on one line, for the snapshot.
    component Fact: RowLayout {
        id: fact
        property string label
        property string value
        property color valueColor: Theme.ink
        Layout.fillWidth: true
        spacing: 10
        Text {
            Layout.fillWidth: true
            text: fact.label
            color: Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
        Text {
            text: fact.value
            color: fact.valueColor
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }
    }

    PageHero {
        overline: "Coach"
        title: "Your coach"
        subtitle: "Suggestions from your own numbers. The plan stays as it is: the coach helps you choose what to do "
                  + "around it, and leans practice towards what you enjoy."
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        ColumnLayout {
            id: leftColumn
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "What to do next"
                subtitle: "Suggestions, not orders"
                icon: "route"

                Text {
                    Layout.fillWidth: true
                    visible: page.tips.length === 0
                    text: "Nothing to suggest right now. ¡Sigue así!"
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }

                Repeater {
                    model: page.tips.slice(0, 12)

                    delegate: ColumnLayout {
                        id: tip
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        spacing: 0

                        Rectangle {
                            visible: tip.index > 0
                            Layout.fillWidth: true
                            Layout.bottomMargin: 14
                            implicitHeight: 1
                            color: Theme.line
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Layout.bottomMargin: tip.index < page.tips.length - 1 ? 14 : 0
                            spacing: 14

                            Rectangle {
                                Layout.alignment: Qt.AlignTop
                                implicitWidth: 38
                                implicitHeight: 38
                                radius: 19
                                color: Theme.accentSoft

                                Icon {
                                    anchors.centerIn: parent
                                    name: page.tipIcon(tip.modelData.id)
                                    size: 18
                                    color: Theme.accentStrong
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 3

                                Text {
                                    Layout.fillWidth: true
                                    text: tip.modelData.title
                                    textFormat: Text.PlainText
                                    color: Theme.ink
                                    font.pixelSize: tip.index === 0 ? 17 : 15
                                    font.weight: Font.DemiBold
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: tip.modelData.detail
                                    textFormat: Text.PlainText
                                    color: Theme.muted
                                    font.pixelSize: 13
                                    wrapMode: Text.WordWrap
                                }
                                ActionLink {
                                    visible: !page.wide && tip.modelData.route !== ""
                                    Layout.topMargin: 4
                                    text: page.actionLabel(tip.modelData.route)
                                    route: tip.modelData.route
                                }
                            }
                            ActionLink {
                                visible: page.wide && tip.modelData.route !== ""
                                Layout.alignment: Qt.AlignVCenter
                                text: page.actionLabel(tip.modelData.route)
                                route: tip.modelData.route
                            }
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                // Only worth it when there is more than one suggestion to choose from.
                visible: page.tips.length > 1
                title: "A manageable focus for today"
                icon: "bolt"

                Repeater {
                    model: page.tips.slice(0, 2)

                    delegate: RowLayout {
                        id: focusRow
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        spacing: 14

                        Text {
                            Layout.preferredWidth: 18
                            text: focusRow.index + 1
                            color: Theme.accent
                            font.pixelSize: 20
                            font.weight: Font.Bold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: focusRow.modelData.title
                            textFormat: Text.PlainText
                            color: Theme.ink
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    text: "These suggestions do not change your study plan."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }

            // Wide: under the suggestions. Narrow: at the end of the page, after the numbers.
            ReflectionCard {
                parent: page.wide ? leftColumn : endSlot
                Layout.fillWidth: true
                learner: page.learner
                asked: true
                activity: "coach"
                askFeel: false
                kinds: page.learner.exerciseKinds
                title: "Tell your coach"
                question: "What do you enjoy most? Pick as many as you like."
            }
        }

        ColumnLayout {
            Layout.fillWidth: !page.wide
            Layout.preferredWidth: page.wide ? 380 : -1
            Layout.alignment: Qt.AlignTop
            spacing: 20

            // The numbers the suggestions rest on, saying so when there are too few to judge.
            Card {
                Layout.fillWidth: true
                title: "Study snapshot"
                subtitle: "Last " + (page.report.windowDays ?? 14) + " days"
                icon: "chart"

                Fact {
                    label: "Cards due for review"
                    value: page.report.due ?? 0
                }
                Fact {
                    label: "Plan pace"
                    value: page.report.pace ?? ""
                    valueColor: page.report.paceTone === "behind" ? Theme.accentStrong
                              : page.report.paceTone === "good" ? Theme.goodInk : Theme.muted
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.topMargin: 2
                    implicitHeight: 1
                    color: Theme.line
                }
                Text {
                    Layout.fillWidth: true
                    text: "First-try answers by skill"
                    color: Theme.ink
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    visible: page.answeredSkills.length === 0
                    text: "No answers in the last " + (page.report.windowDays ?? 14) + " days yet. Practise a little "
                          + "and the coach will see what needs work."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Repeater {
                    model: page.answeredSkills

                    delegate: RowLayout {
                        id: skillRow
                        required property var modelData
                        readonly property bool weak: modelData.enough && modelData.accuracy < page.report.weakBelow
                        Layout.fillWidth: true
                        spacing: 10

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            Text {
                                Layout.fillWidth: true
                                text: skillRow.modelData.label
                                color: Theme.ink
                                font.pixelSize: 14
                                elide: Text.ElideRight
                            }
                            Text {
                                Layout.fillWidth: true
                                text: skillRow.modelData.enough
                                      ? skillRow.modelData.correct + " of " + skillRow.modelData.total + " right"
                                      : "Not enough answers yet (" + skillRow.modelData.total + " of "
                                        + page.report.minAnswers + ")"
                                color: Theme.muted
                                font.pixelSize: 12
                                wrapMode: Text.WordWrap
                            }
                        }
                        Text {
                            text: skillRow.modelData.enough ? skillRow.modelData.accuracy + "%" : "—"
                            color: !skillRow.modelData.enough ? Theme.faint
                                 : skillRow.weak ? Theme.accentStrong : Theme.goodInk
                            font.pixelSize: 17
                            font.weight: Font.Bold
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    Layout.topMargin: 2
                    text: "A skill needs " + (page.report.minAnswers ?? 8) + " answers before the coach judges it; "
                          + "below " + (page.report.weakBelow ?? 70) + "% it suggests some practice."
                    color: Theme.faint
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Card {
                Layout.fillWidth: true
                title: "What your coach knows you enjoy"
                icon: "sparkles"

                Flow {
                    Layout.fillWidth: true
                    visible: (page.report.favourites ?? []).length > 0
                    spacing: 8

                    Repeater {
                        model: page.report.favourites ?? []

                        delegate: Rectangle {
                            required property string modelData
                            implicitWidth: favouriteText.implicitWidth + 24
                            implicitHeight: 30
                            radius: 15
                            color: Theme.accentSoft
                            border.color: Theme.accentLine

                            Text {
                                id: favouriteText
                                anchors.centerIn: parent
                                text: parent.modelData
                                color: Theme.accentStrong
                                font.pixelSize: 13
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: page.learner.tasteText !== ""
                          ? page.learner.tasteText
                          : "Nothing yet. Every " + page.learner.reflectEvery + " sessions you get a quick question about "
                            + "what you enjoyed, and you can tell your coach below at any time."
                    color: page.learner.tasteText !== "" ? Theme.ink : Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
            }

            Card {
                Layout.fillWidth: true
                title: "About the suggestions"
                icon: "lightbulb"

                Text {
                    Layout.fillWidth: true
                    text: "They come from what you have recorded: due cards, plan days and upcoming checkpoints, lesson "
                          + "scores, your answers of the last two weeks, days since you last spoke out loud, how "
                          + "sessions felt and what you enjoy. Fixed rules on this computer work them out; no AI "
                          + "service and no internet are involved."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
        }
    }

    ColumnLayout {
        id: endSlot
        Layout.fillWidth: true
        visible: !page.wide
    }
}
