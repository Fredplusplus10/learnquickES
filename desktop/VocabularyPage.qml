import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Every word of the course by lesson, with its article colour, audio, example and where it stands in review.
// The four figures at the top say where the words stand and filter the list.
ScrollPage {
    id: page

    required property LearnerController learner
    maxWidth: 1240

    readonly property real columnWidth: Math.min(width - 64, maxWidth)

    property var words: learner.vocabulary()
    property string query: ""
    property int phase: 0          // 0 for every phase
    property string status: "all"  // "all", "due", "learning", "remembered" or "new"

    readonly property var statusLabels: ({ due: "Due today", learning: "Learning", remembered: "Remembered", new: "Not studied" })
    // One width for every review badge, so the examples line up from row to row.
    readonly property real badgeWidth: Math.ceil(Math.max(dueLabel.advanceWidth, learningLabel.advanceWidth,
                                                          rememberedLabel.advanceWidth, newLabel.advanceWidth)) + 16

    component BadgeMetrics: TextMetrics {
        font.pixelSize: 12
        font.weight: Font.DemiBold
    }
    BadgeMetrics { id: dueLabel; text: page.statusLabels.due }
    BadgeMetrics { id: learningLabel; text: page.statusLabels.learning }
    BadgeMetrics { id: rememberedLabel; text: page.statusLabels.remembered }
    BadgeMetrics { id: newLabel; text: page.statusLabels["new"] }

    readonly property bool filtered: query.trim() !== "" || phase !== 0 || status !== "all"
    function clearFilters() {
        search.text = ""
        phase = 0
        status = "all"
    }

    // The colours of each review state, shared by the figures and the badges.
    function stateFill(state) {
        switch (state) {
        case "due": return Theme.accentSoft
        case "remembered": return Theme.successSoft
        case "learning": return Theme.blueSoft
        default: return Theme.background
        }
    }
    function stateInk(state) {
        switch (state) {
        case "due": return Theme.accentStrong
        case "remembered": return Theme.goodInk
        case "learning": return Theme.blue
        default: return Theme.muted
        }
    }

    function count(state) {
        return words.filter(w => w.state === state).length
    }

    readonly property int deckCount: words.length - count("new")
    readonly property int dueCount: count("due")

    // The words that pass the filters, grouped by lesson.
    readonly property var groups: {
        const key = query.trim() === "" ? "" : learner.searchKey(query)
        const out = []
        for (const w of words) {
            if (phase !== 0 && w.phase !== phase)
                continue
            if (status !== "all" && w.state !== status)
                continue
            if (key !== "" && w.key.indexOf(key) < 0)
                continue
            if (out.length === 0 || out[out.length - 1].unit !== w.unit)
                out.push({ unit: w.unit, lesson: w.lesson, title: w.lessonTitle, words: [] })
            out[out.length - 1].words.push(w)
        }
        return out
    }
    readonly property int shown: groups.reduce((n, g) => n + g.words.length, 0)

    // A review graded elsewhere, or a lesson finished, changes where words stand.
    Connections {
        target: page.learner
        function onChanged() { page.words = page.learner.vocabulary() }
    }

    // The article colours, beside the search box on a wide page and under the phases on a narrow one.
    component GenderLegend: Text {
        text: "<font color=\"" + Theme.masculine + "\"><b>el</b></font> masculine · <font color=\"" + Theme.feminine
              + "\"><b>la</b></font> feminine"
        textFormat: Text.StyledText
        color: Theme.muted
        font.pixelSize: 13
    }

    PageHero {
        overline: "Your words"
        title: "Words"
        subtitle: page.deckCount === 0
                  ? "Every word of the course by lesson, with its audio and an example. Words join your review deck "
                    + "when you finish their lesson."
                  : page.deckCount + " of " + page.words.length + " words are in your review deck. Every word of the "
                    + "course is here by lesson, with its audio, an example and where it stands."

        PrimaryButton {
            visible: page.dueCount > 0
            text: "Review " + page.dueCount + " due"
            onClicked: page.openRoute("practice/review")
        }
        PrimaryButton {
            visible: page.deckCount === 0
            text: "Open lessons"
            onClicked: page.openRoute("lessons")
        }
    }

    // Where the words stand. Each figure filters the list; a second click shows every word again.
    GridLayout {
        Layout.fillWidth: true
        columns: page.columnWidth >= 760 ? 4 : 2
        rowSpacing: 12
        columnSpacing: 12

        Repeater {
            model: [
                { state: "due", icon: "clock", note: "Ready to review today" },
                { state: "learning", icon: "refresh", note: "Back within a few days" },
                { state: "remembered", icon: "check", note: "Next review a week or more away" },
                { state: "new", icon: "book", note: "In lessons not finished yet" }
            ]

            delegate: AbstractButton {
                id: figure
                required property var modelData
                readonly property bool active: page.status === modelData.state
                readonly property int amount: page.count(modelData.state)

                Layout.fillWidth: true
                Layout.preferredWidth: 1
                implicitHeight: figureColumn.implicitHeight + 28
                focusPolicy: Qt.StrongFocus
                onClicked: page.status = active ? "all" : modelData.state
                Keys.onReturnPressed: clicked()
                Keys.onEnterPressed: clicked()
                Accessible.name: amount + " " + page.statusLabels[modelData.state]
                                 + (active ? ", shown. Click to show every word" : ". Click to show only these")

                background: Rectangle {
                    radius: Theme.radius
                    color: figure.active ? page.stateFill(figure.modelData.state)
                                         : figure.hovered ? Theme.hover : Theme.panel
                    border.color: figure.visualFocus ? Theme.accent
                                  : figure.active ? page.stateInk(figure.modelData.state)
                                  : figure.hovered ? Theme.hoverLine : Theme.line
                    border.width: figure.visualFocus || figure.active ? 2 : 1
                }
                contentItem: Item {
                    ColumnLayout {
                        id: figureColumn
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 18 }
                        spacing: 2

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Text {
                                Layout.fillWidth: true
                                text: page.statusLabels[figure.modelData.state]
                                color: Theme.muted
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }
                            Icon {
                                name: figure.modelData.icon
                                size: 16
                                color: page.stateInk(figure.modelData.state)
                            }
                        }
                        Text {
                            text: figure.amount
                            color: figure.modelData.state === "new" ? Theme.ink : page.stateInk(figure.modelData.state)
                            font.pixelSize: 28
                            font.weight: Font.Bold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: figure.modelData.note
                            color: Theme.faint
                            font.pixelSize: 12
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true

        RowLayout {
            Layout.fillWidth: true
            spacing: 16

            InputField {
                id: search
                Layout.fillWidth: true
                leftPadding: 40
                placeholderText: "Search Spanish or English…"
                onTextChanged: page.query = text

                Icon {
                    anchors.left: parent.left
                    anchors.leftMargin: 13
                    anchors.verticalCenter: parent.verticalCenter
                    name: "search"
                    size: 17
                    color: Theme.faint
                }
            }
            GenderLegend {
                visible: page.columnWidth >= 760
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [0, 1, 2, 3, 4, 5]

                delegate: Chip {
                    required property int modelData
                    text: modelData === 0 ? "All phases" : "Phase " + modelData
                    checked: page.phase === modelData
                    onClicked: page.phase = modelData
                }
            }
        }
        GenderLegend {
            visible: page.columnWidth < 760
        }
        // What the list shows, and the way back to every word.
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: page.filtered ? (page.shown === 1 ? "1 word matches" : page.shown + " words match")
                                      + (page.status !== "all" ? " · " + page.statusLabels[page.status].toLowerCase() : "")
                                    : "All " + page.words.length + " words of the course"
                color: Theme.muted
                font.pixelSize: 13
            }
            AbstractButton {
                id: clearLink
                visible: page.filtered
                focusPolicy: Qt.StrongFocus
                onClicked: page.clearFilters()
                Keys.onReturnPressed: clicked()
                Keys.onEnterPressed: clicked()
                Accessible.name: "Show every word"
                background: null
                contentItem: RowLayout {
                    spacing: 4
                    Icon {
                        name: "x"
                        size: 13
                        color: clearLink.hovered || clearLink.visualFocus ? Theme.accentStrong : Theme.accent
                    }
                    Text {
                        text: "Show every word"
                        color: clearLink.hovered || clearLink.visualFocus ? Theme.accentStrong : Theme.accent
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        font.underline: clearLink.visualFocus
                    }
                }
            }
        }
    }

    Repeater {
        model: page.groups

        delegate: Card {
            id: group
            required property var modelData
            Layout.fillWidth: true
            title: "Lesson " + modelData.lesson + " · " + modelData.title
            subtitle: modelData.words.length === 1 ? "1 word" : modelData.words.length + " words"

            Repeater {
                model: group.modelData.words

                delegate: ColumnLayout {
                    id: line
                    required property var modelData
                    required property int index
                    Layout.fillWidth: true
                    spacing: 10

                    Rectangle {
                        visible: line.index > 0
                        Layout.fillWidth: true
                        implicitHeight: 1
                        color: Theme.line
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        SpeakButton {
                            spanish: line.modelData.es
                            diameter: 32
                        }
                        // The word and the example share the row 2 : 3, whatever their text's natural width.
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 2
                            spacing: 2

                            SpanishWord {
                                Layout.fillWidth: true
                                spanish: line.modelData.es
                                gender: line.modelData.gender
                                font.pixelSize: 16
                            }
                            Text {
                                Layout.fillWidth: true
                                text: line.modelData.en
                                color: Theme.muted
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                                textFormat: Text.PlainText
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.preferredWidth: 3
                            visible: page.width > 700 && line.modelData.example !== ""
                            text: line.modelData.example
                            color: Theme.muted
                            font.pixelSize: 13
                            font.italic: true
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                        Rectangle {
                            Layout.alignment: Qt.AlignVCenter
                            implicitWidth: page.badgeWidth
                            implicitHeight: 22
                            radius: 11
                            color: page.stateFill(line.modelData.state)
                            border.color: line.modelData.state === "new" ? Theme.line : "transparent"

                            Text {
                                anchors.centerIn: parent
                                text: page.statusLabels[line.modelData.state]
                                color: page.stateInk(line.modelData.state)
                                font.pixelSize: 12
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                }
            }
        }
    }

    Card {
        visible: page.groups.length === 0
        Layout.fillWidth: true

        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: 10
            Layout.bottomMargin: 10
            spacing: 8

            Image {
                Layout.alignment: Qt.AlignHCenter
                source: "assets/illustrations/" + (page.status === "due" && page.query.trim() === "" ? "empty-caught-up"
                                                                                                   : "empty-search") + ".svg"
                sourceSize: Qt.size(160, 120)
                Layout.preferredWidth: 160
                Layout.preferredHeight: 120
            }
            Text {
                Layout.fillWidth: true
                text: page.status === "due" && page.query.trim() === "" ? "Nothing due here. ¡Muy bien!" : "No words match"
                color: Theme.ink
                font.pixelSize: 16
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
            }
            Text {
                Layout.fillWidth: true
                text: "Try another search, phase or figure above."
                color: Theme.muted
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }
            PrimaryButton {
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 4
                secondary: true
                text: "Show every word"
                onClicked: page.clearFilters()
            }
        }
    }
}
