import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// First run, always in English: who the learner is, why they want Spanish, and the plan that fits.
// The reason, in their own words, is stored with the profile and shown back on the Today screen.
Item {
    id: welcome

    required property ProfileController profile
    property int step: 0

    property string level: "A2"
    property int planDays: 60
    property bool planPicked: false
    property var goals: []

    readonly property string trimmedName: nameField.text.trim()
    readonly property int reasonLength: reasonArea.text.trim().length
    readonly property bool reasonReady: reasonLength >= profile.minReasonLength
    readonly property int recommendedDays: profile.recommendedPlanDays(level)

    // Follow the recommendation until the learner picks a plan themselves.
    onRecommendedDaysChanged: if (!planPicked) planDays = recommendedDays
    onStepChanged: focusStep()
    Component.onCompleted: {
        planDays = recommendedDays
        focusStep()
    }

    function focusStep() {
        if (step === 0)
            nameField.forceActiveFocus()
        else if (step === 1)
            reasonArea.forceActiveFocus()
    }

    function toggleGoal(id: string) {
        const next = goals.slice()
        const at = next.indexOf(id)
        if (at >= 0)
            next.splice(at, 1)
        else
            next.push(id)
        goals = next
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Brand and progress through the three steps.
        Rectangle {
            Layout.preferredWidth: 330
            Layout.fillHeight: true
            color: Theme.sidebar

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 36
                spacing: 0

                RowLayout {
                    spacing: 12

                    Image {
                        source: "images/learnquick.png"
                        Layout.preferredWidth: 44
                        Layout.preferredHeight: 44
                        mipmap: true
                    }
                    Text {
                        text: "LearnQuick"
                        color: Theme.ink
                        font.pixelSize: 22
                        font.weight: Font.Bold
                    }
                }

                Text {
                    Layout.topMargin: 24
                    Layout.fillWidth: true
                    text: "Spanish to a functional B1 in 30 to 90 days."
                    color: Theme.muted
                    font.pixelSize: 15
                    wrapMode: Text.WordWrap
                }

                ColumnLayout {
                    Layout.topMargin: 44
                    spacing: 18

                    Repeater {
                        model: ["About you", "Your why", "Your plan"]

                        delegate: RowLayout {
                            id: stepRow
                            required property string modelData
                            required property int index
                            readonly property bool current: index === welcome.step
                            spacing: 14

                            Rectangle {
                                implicitWidth: 30
                                implicitHeight: 30
                                radius: 15
                                color: stepRow.index < welcome.step ? Theme.success
                                     : stepRow.current ? Theme.accent : "transparent"
                                border.color: stepRow.index > welcome.step ? Theme.faint : "transparent"
                                border.width: 1.5

                                Text {
                                    anchors.centerIn: parent
                                    text: stepRow.index + 1
                                    color: stepRow.index > welcome.step ? Theme.muted : Theme.inkOnAccent
                                    font.pixelSize: 13
                                    font.weight: Font.DemiBold
                                }
                            }
                            Text {
                                text: stepRow.modelData
                                color: stepRow.current ? Theme.ink : Theme.muted
                                font.pixelSize: 15
                                font.weight: stepRow.current ? Font.DemiBold : Font.Normal
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                Text {
                    Layout.fillWidth: true
                    text: "Everything you write here stays on this computer."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
        }

        // The current step. Hidden steps keep what was typed, so Back never loses an answer.
        Item {
            id: stage
            Layout.fillWidth: true
            Layout.fillHeight: true
            readonly property real pageWidth: Math.min(width - 96, 640)

            ColumnLayout {
                visible: welcome.step === 0
                anchors.centerIn: parent
                width: stage.pageWidth
                spacing: 14

                Text {
                    Layout.fillWidth: true
                    text: "Welcome! What should we call you?"
                    color: Theme.ink
                    font.pixelSize: 30
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "LearnQuick builds a daily plan that takes you to a conversational B1. "
                          + "Three quick questions first, so the plan fits you."
                    color: Theme.muted
                    font.pixelSize: 16
                    lineHeight: 1.2
                    wrapMode: Text.WordWrap
                }
                TextField {
                    id: nameField
                    Layout.fillWidth: true
                    Layout.topMargin: 14
                    placeholderText: "Your first name"
                    placeholderTextColor: Theme.faint
                    maximumLength: welcome.profile.maxNameLength
                    color: Theme.ink
                    font.pixelSize: 17
                    padding: 14
                    onAccepted: if (welcome.trimmedName !== "") welcome.step = 1

                    background: Rectangle {
                        radius: Theme.smallRadius
                        color: Theme.panel
                        border.color: nameField.activeFocus ? Theme.accent : Theme.line
                        border.width: nameField.activeFocus ? 2 : 1
                    }
                }
                RowLayout {
                    Layout.topMargin: 10

                    Item {
                        Layout.fillWidth: true
                    }
                    PrimaryButton {
                        text: "Continue"
                        enabled: welcome.trimmedName !== ""
                        onClicked: welcome.step = 1
                    }
                }
            }

            ColumnLayout {
                visible: welcome.step === 1
                anchors.centerIn: parent
                width: stage.pageWidth
                spacing: 14

                Text {
                    Layout.fillWidth: true
                    text: welcome.trimmedName !== "" ? "Why do you want to learn Spanish, " + welcome.trimmedName + "?"
                                                     : "Why do you want to learn Spanish?"
                    textFormat: Text.PlainText
                    color: Theme.ink
                    font.pixelSize: 30
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "Be specific: who do you want to talk to, and about what? Your answer shapes your "
                          + "practice and keeps you going on the hard days."
                    color: Theme.muted
                    font.pixelSize: 16
                    lineHeight: 1.2
                    wrapMode: Text.WordWrap
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    Layout.topMargin: 8

                    background: Rectangle {
                        radius: Theme.smallRadius
                        color: Theme.panel
                        border.color: reasonArea.activeFocus ? Theme.accent : Theme.line
                        border.width: reasonArea.activeFocus ? 2 : 1
                    }

                    TextArea {
                        id: reasonArea
                        placeholderText: "For example: my partner's family speaks Spanish, and I want to follow "
                                         + "the conversation at dinner and join in."
                        placeholderTextColor: Theme.faint
                        color: Theme.ink
                        font.pixelSize: 16
                        wrapMode: TextArea.Wrap
                        padding: 14
                        background: null
                        // TextArea has no maximumLength; trim anything pasted past the limit.
                        onTextChanged: if (length > welcome.profile.maxReasonLength)
                                           remove(welcome.profile.maxReasonLength, length)
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: welcome.reasonReady ? welcome.reasonLength + " characters"
                                              : welcome.reasonLength + " / " + welcome.profile.minReasonLength
                                                + " characters minimum"
                    color: welcome.reasonReady ? Theme.success : Theme.muted
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignRight
                }
                Text {
                    text: "What is it for? Pick any that apply."
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: [
                            { id: "travel", label: "Travel" },
                            { id: "work", label: "Work" },
                            { id: "family", label: "Family and friends" },
                            { id: "moving", label: "Moving abroad" },
                            { id: "culture", label: "Culture and media" },
                            { id: "challenge", label: "Personal challenge" }
                        ]

                        delegate: Chip {
                            required property var modelData
                            text: modelData.label
                            checked: welcome.goals.indexOf(modelData.id) >= 0
                            onClicked: welcome.toggleGoal(modelData.id)
                        }
                    }
                }
                RowLayout {
                    Layout.topMargin: 10

                    PrimaryButton {
                        text: "Back"
                        secondary: true
                        onClicked: welcome.step = 0
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    PrimaryButton {
                        text: "Continue"
                        enabled: welcome.reasonReady
                        onClicked: welcome.step = 2
                    }
                }
            }

            ColumnLayout {
                visible: welcome.step === 2
                anchors.centerIn: parent
                width: stage.pageWidth
                spacing: 12

                Text {
                    Layout.fillWidth: true
                    text: "Where are you starting from?"
                    color: Theme.ink
                    font.pixelSize: 30
                    font.weight: Font.Bold
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "Pick the level that sounds most like you today."
                    color: Theme.muted
                    font.pixelSize: 16
                    wrapMode: Text.WordWrap
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    spacing: 12

                    Repeater {
                        model: [
                            { level: "A1", heading: "A1 · Beginner", detail: "I know some words and phrases." },
                            { level: "A2", heading: "A2 · Elementary", detail: "I can handle simple conversations." },
                            { level: "A2+", heading: "A2+ · Almost B1", detail: "I talk about the past, with some errors." }
                        ]

                        delegate: ChoiceCard {
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.preferredWidth: 1
                            heading: modelData.heading
                            detail: modelData.detail
                            checked: welcome.level === modelData.level
                            onClicked: welcome.level = modelData.level
                        }
                    }
                }
                Text {
                    Layout.topMargin: 10
                    text: "Choose your plan"
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Repeater {
                        model: [
                            { days: 30, heading: "30 days", detail: "Intensive. For solid A2: a lesson every day and a lot of listening." },
                            { days: 60, heading: "60 days", detail: "Balanced. For most A2 learners: lessons with practice days between." },
                            { days: 90, heading: "90 days", detail: "Steady. From A1 or after a long break: more practice, calmer pace." }
                        ]

                        delegate: ChoiceCard {
                            required property var modelData
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Layout.preferredWidth: 1
                            heading: modelData.heading
                            detail: modelData.detail
                            badge: modelData.days === welcome.recommendedDays ? "Recommended" : ""
                            checked: welcome.planDays === modelData.days
                            onClicked: {
                                welcome.planDays = modelData.days
                                welcome.planPicked = true
                            }
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: "Your plan starts today. Weekends and US federal holidays are days off, and they never "
                          + "break your streak."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    visible: text !== ""
                    text: welcome.profile.error
                    textFormat: Text.PlainText
                    color: Theme.accentStrong
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
                RowLayout {
                    Layout.topMargin: 6

                    PrimaryButton {
                        text: "Back"
                        secondary: true
                        onClicked: welcome.step = 1
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    PrimaryButton {
                        text: "Start learning"
                        onClicked: welcome.profile.save(welcome.trimmedName, reasonArea.text, welcome.goals,
                                                        welcome.level, welcome.planDays)
                    }
                }
            }
        }
    }
}
