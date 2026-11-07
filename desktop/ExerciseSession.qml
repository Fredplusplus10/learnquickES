import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// Plays the session SessionModel holds, one item at a time, then the second-try round of the first misses.
// The owner shows the summary once session.finished turns true. Keyboard: Enter checks and continues,
// 1–9 pick, Space reveals.
FocusScope {
    id: root

    required property SessionModel session

    readonly property var item: session.item
    readonly property string type: item.type ?? ""
    readonly property int itemKey: item.key ?? -1
    readonly property bool checkable: ["choice", "fill", "type", "conj", "order", "listen"].indexOf(type) >= 0
    readonly property var feedback: session.feedback
    // A level assessment shows no marks while it runs: an answer is given, not checked.
    readonly property string checkText: session.kind === "assessment" ? "Answer" : "Check"

    implicitHeight: session.intermission ? secondTryIntro.implicitHeight : column.implicitHeight

    function primary() {
        if (!session.active || session.finished)
            return
        if (session.intermission) {
            session.startSecondTry()
            return
        }
        if (session.checked) {
            session.next()
            return
        }
        switch (type) {
        case "card":
            if (!session.revealed)
                session.reveal()
            else
                session.grade(2)
            break
        case "speak":
            session.selfRate(true)
            break
        case "describe":
            if (!session.revealed)
                session.reveal()
            break
        default:
            if (session.canCheck)
                session.check()
        }
    }

    // A fresh view per item, so text fields and tiles never carry over.
    onItemKeyChanged: {
        body.active = false
        body.active = true
        Qt.callLater(root.forceActiveFocus)
    }
    Component.onCompleted: root.forceActiveFocus()

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
            root.primary()
            event.accepted = true
        } else if (event.key === Qt.Key_Space) {
            if ((type === "card" || type === "describe") && !session.revealed) {
                session.reveal()
                event.accepted = true
            }
        } else if (event.key === Qt.Key_Backspace && (type === "order" || type === "listen") && !session.checked) {
            const picked = session.picked
            if (picked.length > 0)
                session.unpick(picked[picked.length - 1])
            event.accepted = true
        } else if (event.key >= Qt.Key_1 && event.key <= Qt.Key_9) {
            const n = event.key - Qt.Key_1
            if (type === "choice")
                session.select(n)
            else if (type === "gender" && n < 2)
                session.chooseArticle(item.articles[n])
            else if (type === "card" && session.revealed && n < 3)
                session.grade([0, 2, 3][n])
            else if (type === "describe" && session.revealed && n < 2)
                session.selfRate(n === 0)
            else
                return
            event.accepted = true
        }
    }

    // ---- Building blocks --------------------------------------------------------------------------

    component Prompt: Text {
        Layout.fillWidth: true
        color: Theme.ink
        font.pixelSize: 22
        font.weight: Font.DemiBold
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
    }

    component Muted: Text {
        Layout.fillWidth: true
        color: Theme.muted
        font.pixelSize: 14
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
    }

    component AnswerField: TextField {
        id: field
        font.pixelSize: 19
        readOnly: root.session.checked
        color: Theme.ink
        selectByMouse: true
        leftPadding: 12
        rightPadding: 12
        inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase
        onTextChanged: root.session.setInput(text)
        background: Rectangle {
            implicitHeight: 46
            radius: Theme.smallRadius
            color: !root.session.checked ? Theme.panel : root.feedback.ok ? Theme.successSoft : Theme.dangerSoft
            border.color: root.session.checked ? (root.feedback.ok ? Theme.success : Theme.danger)
                                               : field.activeFocus ? Theme.accent : Theme.line
            border.width: field.activeFocus || root.session.checked ? 2 : 1
        }
    }

    // On-screen Enter does what the physical one does: check, then continue. It carries the label, so the
    // main action stays next to the keys even when the window is too short to show the buttons below.
    component AnswerKeyboard: SpanishKeyboard {
        Layout.fillWidth: true
        Layout.maximumWidth: implicitWidth
        Layout.topMargin: 4
        enterText: root.session.checked ? "Continue" : root.checkText
        enterEnabled: root.session.checked || root.session.canCheck
        onSubmitted: root.primary()
    }

    component Tile: AbstractButton {
        id: tile
        property bool placeholder: false
        implicitWidth: implicitContentWidth + leftPadding + rightPadding
        implicitHeight: 42
        leftPadding: 14
        rightPadding: 14
        focusPolicy: Qt.NoFocus
        enabled: !placeholder && !root.session.checked
        contentItem: Text {
            text: tile.text
            opacity: tile.placeholder ? 0 : 1
            color: Theme.ink
            font.pixelSize: 17
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 10
            color: tile.placeholder ? Theme.sidebar : tile.hovered ? Theme.hover : Theme.raised
            border.color: tile.placeholder ? Theme.line : tile.hovered ? Theme.hoverLine : Theme.edge
            border.width: 1.5
        }
    }

    // A choice with its keyboard number; after checking, green for the right one and red for a wrong pick.
    component Option: AbstractButton {
        id: option
        property int number: 0
        property bool selected: false
        property bool correct: false
        readonly property bool reveal: root.session.checked
        implicitHeight: Math.max(52, label.implicitHeight + 24)
        focusPolicy: Qt.NoFocus
        enabled: !root.session.checked
        contentItem: RowLayout {
            spacing: 12
            Rectangle {
                implicitWidth: 26
                implicitHeight: 26
                radius: 7
                color: Theme.panel
                border.color: Theme.line
                Text {
                    anchors.centerIn: parent
                    text: option.number
                    color: Theme.muted
                    font.pixelSize: 13
                }
            }
            Text {
                id: label
                Layout.fillWidth: true
                text: option.text
                color: Theme.ink
                font.pixelSize: 17
                wrapMode: Text.WordWrap
                textFormat: Text.PlainText
            }
            Icon {
                visible: option.reveal && (option.correct || option.selected)
                name: option.correct ? "check" : "x"
                color: option.correct ? Theme.success : Theme.danger
            }
        }
        leftPadding: 14
        rightPadding: 14
        background: Rectangle {
            radius: Theme.smallRadius
            color: option.reveal && option.correct ? Theme.successSoft
                 : option.reveal && option.selected ? Theme.dangerSoft
                 : option.selected ? Theme.accentSoft : option.hovered ? Theme.hover : Theme.raised
            border.color: option.reveal && option.correct ? Theme.success
                        : option.reveal && option.selected ? Theme.danger
                        : option.selected ? Theme.accent : Theme.edge
            border.width: option.selected || (option.reveal && option.correct) ? 2 : 1.5
        }
    }

    // One side of a match pair.
    component Cell: AbstractButton {
        id: cell
        property string side
        property int pair: -1
        readonly property bool done: root.session.matched.indexOf(pair) >= 0
        readonly property bool active: root.session.matchSide === side && root.session.matchIndex === pair
        readonly property bool miss: root.session.matchMiss[side] === pair
        Layout.fillWidth: true
        implicitHeight: 48
        focusPolicy: Qt.NoFocus
        enabled: !done
        onClicked: root.session.tapMatch(side, pair)
        contentItem: Text {
            text: cell.text
            color: cell.done ? Theme.goodInk : Theme.ink
            font.pixelSize: 16
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            radius: Theme.smallRadius
            color: cell.done ? Theme.successSoft : cell.miss ? Theme.dangerSoft : cell.active ? Theme.accentSoft
                 : cell.hovered ? Theme.hover : Theme.raised
            border.color: cell.done ? Theme.goodLine : cell.miss ? Theme.danger : cell.active ? Theme.accent : Theme.edge
            border.width: cell.active || cell.miss ? 2 : 1.5
        }
    }

    // ---- Layout -----------------------------------------------------------------------------------

    // Between the rounds: what the second try is worth, before it starts.
    ColumnLayout {
        id: secondTryIntro
        visible: root.session.intermission
        // The answer field of the last item may have had the focus; Enter starts the round.
        onVisibleChanged: if (visible) root.forceActiveFocus()
        anchors { left: parent.left; right: parent.right }
        spacing: 18

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: introColumn.implicitHeight + 48
            radius: Theme.radius
            color: Theme.blueSoft
            border.color: Theme.blue

            ColumnLayout {
                id: introColumn
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 24 }
                spacing: 10

                RowLayout {
                    spacing: 10
                    Icon {
                        name: "refresh"
                        color: Theme.blue
                        size: 26
                    }
                    Text {
                        text: "Second try"
                        color: Theme.ink
                        font.pixelSize: 24
                        font.weight: Font.Bold
                    }
                }
                Text {
                    Layout.fillWidth: true
                    text: (root.session.secondTryCount === 1 ? "One answer" : root.session.secondTryCount + " answers")
                          + " to fix. Each one you get right now is worth +" + root.session.secondTryXp + " XP."
                    color: Theme.ink
                    font.pixelSize: 16
                    wrapMode: Text.WordWrap
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            PrimaryButton {
                secondary: true
                text: "Finish now"
                focusPolicy: Qt.NoFocus
                onClicked: root.session.skipSecondTry()
            }
            Item {
                Layout.fillWidth: true
            }
            PrimaryButton {
                text: "Start second try"
                focusPolicy: Qt.NoFocus
                onClicked: root.session.startSecondTry()
            }
        }
    }

    ColumnLayout {
        id: column
        visible: !root.session.intermission
        anchors { left: parent.left; right: parent.right }
        spacing: 18

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 8
                radius: 4
                color: Theme.line

                Rectangle {
                    width: root.session.count > 0 ? parent.width * root.session.position / root.session.count : 0
                    height: parent.height
                    radius: 4
                    color: root.session.secondTry ? Theme.blue : Theme.accent
                    Behavior on width { NumberAnimation { duration: 200 } }
                }
            }
            Text {
                text: Math.min(root.session.position + 1, root.session.count) + " / " + root.session.count
                color: Theme.muted
                font.pixelSize: 13
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: (root.item.instruction ?? "").toUpperCase()
                color: Theme.muted
                font.pixelSize: 12
                font.weight: Font.DemiBold
                font.letterSpacing: 0.7
            }
            Rectangle {
                visible: root.item.retry ?? false
                implicitWidth: retryText.implicitWidth + 16
                implicitHeight: 22
                radius: 11
                color: Theme.blueSoft
                Text {
                    id: retryText
                    anchors.centerIn: parent
                    text: root.type === "card" ? "Second try" : "Second try · +" + root.session.secondTryXp + " XP"
                    color: Theme.blue
                    font.pixelSize: 12
                }
            }
        }

        Loader {
            id: body
            Layout.fillWidth: true
            sourceComponent: {
                switch (root.type) {
                case "choice": return choiceView
                case "fill": return fillView
                case "type": return typeView
                case "conj": return conjView
                case "order":
                case "listen": return tilesView
                case "match": return matchView
                case "gender": return genderView
                case "card": return cardView
                case "speak": return speakView
                case "describe": return describeView
                default: return null
                }
            }
        }

        // What the check found, with the correct answer and the reason when it was wrong.
        Rectangle {
            visible: root.session.checked
            Layout.fillWidth: true
            implicitHeight: feedbackColumn.implicitHeight + 28
            radius: Theme.smallRadius
            color: root.feedback.ok ? Theme.successSoft : Theme.dangerSoft
            border.color: root.feedback.ok ? Theme.goodLine : Theme.badLine

            ColumnLayout {
                id: feedbackColumn
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
                spacing: 6

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10

                    Icon {
                        name: root.feedback.ok ? "check" : "x"
                        color: root.feedback.ok ? Theme.goodInk : Theme.badInk
                    }
                    Text {
                        Layout.fillWidth: true
                        text: root.feedback.ok ? (root.feedback.almost ? "Almost! Watch the accents: " + (root.feedback.expected ?? "")
                                                                       : root.feedback.praise ?? "")
                                               : "Correct answer: " + (root.feedback.expected ?? "")
                        color: root.feedback.ok ? Theme.goodInk : Theme.badInk
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                    SpeakButton {
                        spanish: root.feedback.sentence ?? ""
                        visible: Speech.available && spanish !== ""
                        diameter: 32
                    }
                }
                Text {
                    visible: text !== ""
                    Layout.fillWidth: true
                    text: root.feedback.why ?? ""
                    color: root.feedback.ok ? Theme.goodInk : Theme.badInk
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                Text {
                    visible: text !== ""
                    Layout.fillWidth: true
                    text: root.feedback.note ?? ""
                    color: root.feedback.ok ? Theme.goodInk : Theme.badInk
                    font.pixelSize: 14
                    font.italic: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                Text {
                    visible: root.feedback.requeued ?? false
                    text: "You'll get a second try at the end, worth +" + root.session.secondTryXp + " XP."
                    color: Theme.badInk
                    font.pixelSize: 13
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12
            visible: root.checkable || root.type === "gender" || root.session.checked

            PrimaryButton {
                secondary: true
                text: "I don't know"
                focusPolicy: Qt.NoFocus
                visible: !root.session.checked && (root.checkable || root.type === "gender")
                onClicked: root.session.skip()
            }
            Item {
                Layout.fillWidth: true
            }
            PrimaryButton {
                text: root.checkText
                focusPolicy: Qt.NoFocus
                visible: root.checkable && !root.session.checked
                enabled: root.session.canCheck
                onClicked: root.session.check()
            }
            PrimaryButton {
                text: "Continue"
                focusPolicy: Qt.NoFocus
                visible: root.session.checked
                onClicked: root.session.next()
            }
        }
    }

    // ---- One view per exercise type ---------------------------------------------------------------

    Component {
        id: choiceView

        ColumnLayout {
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                Layout.bottomMargin: 6
                spacing: 12

                SpeakButton {
                    spanish: root.item.audio ?? ""
                    visible: Speech.available && spanish !== ""
                }
                Prompt {
                    text: root.item.question ?? ""
                }
            }
            Repeater {
                model: root.item.options ?? []

                delegate: Option {
                    required property string modelData
                    required property int index
                    Layout.fillWidth: true
                    text: modelData
                    number: index + 1
                    selected: root.session.selected === index
                    correct: index === (root.item.answer ?? -1)
                    onClicked: root.session.select(index)
                }
            }
        }
    }

    Component {
        id: fillView

        ColumnLayout {
            spacing: 14

            Flow {
                Layout.fillWidth: true
                spacing: 8

                Text {
                    width: Math.min(implicitWidth, parent.width)
                    height: fillField.height
                    verticalAlignment: Text.AlignVCenter
                    text: root.item.before ?? ""
                    color: Theme.ink
                    font.pixelSize: 21
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                AnswerField {
                    id: fillField
                    width: Math.max(110, (root.item.width ?? 8) * 13)
                    Component.onCompleted: forceActiveFocus()
                }
                Text {
                    width: Math.min(implicitWidth, parent.width)
                    height: fillField.height
                    verticalAlignment: Text.AlignVCenter
                    text: root.item.after ?? ""
                    color: Theme.ink
                    font.pixelSize: 21
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
            }
            Muted {
                visible: text !== ""
                text: (root.item.hint ?? "") !== "" ? "Hint: " + root.item.hint : ""
            }
            AnswerKeyboard {
                target: fillField
            }
        }
    }

    Component {
        id: typeView

        ColumnLayout {
            spacing: 14

            Muted {
                text: root.item.prompt ?? ""
            }
            Prompt {
                text: root.item.question ?? ""
                font.pixelSize: 26
            }
            AnswerField {
                id: typeField
                Layout.preferredWidth: 360
                Component.onCompleted: forceActiveFocus()
            }
            AnswerKeyboard {
                target: typeField
            }
        }
    }

    Component {
        id: conjView

        ColumnLayout {
            spacing: 14

            RowLayout {
                spacing: 10

                Text {
                    text: root.item.verb ?? ""
                    color: Theme.ink
                    font.pixelSize: 28
                    font.weight: Font.Bold
                }
                Text {
                    text: (root.item.verbEn ?? "") !== "" ? "(" + root.item.verbEn + ")" : ""
                    color: Theme.muted
                    font.pixelSize: 16
                }
            }
            Rectangle {
                implicitWidth: tenseText.implicitWidth + 24
                implicitHeight: 30
                radius: 15
                color: Theme.purpleSoft

                Text {
                    id: tenseText
                    anchors.centerIn: parent
                    text: (root.item.tense ?? "") + ((root.item.cue ?? "") !== "" ? " · " + root.item.cue : "")
                    color: Theme.purple
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }
            }
            RowLayout {
                spacing: 12

                Text {
                    text: root.item.pronoun ?? ""
                    color: Theme.ink
                    font.pixelSize: 21
                }
                AnswerField {
                    id: conjField
                    Layout.preferredWidth: Math.max(140, (root.item.width ?? 8) * 14)
                    Component.onCompleted: forceActiveFocus()
                }
            }
            AnswerKeyboard {
                target: conjField
            }
        }
    }

    Component {
        id: tilesView

        ColumnLayout {
            id: tiles
            readonly property bool listening: root.type === "listen"
            readonly property var picked: root.session.picked
            spacing: 16

            Component.onCompleted: if (listening) Speech.say(root.item.es ?? "")

            RowLayout {
                visible: tiles.listening
                spacing: 12

                SpeakButton {
                    spanish: root.item.es ?? ""
                    diameter: 60
                }
                SpeakButton {
                    spanish: root.item.es ?? ""
                    slow: true
                    diameter: 44
                }
            }
            Prompt {
                visible: !tiles.listening
                text: "“" + (root.item.en ?? "") + "”"
            }

            // The sentence so far: tap a tile to send it back.
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: Math.max(64, answerFlow.implicitHeight + 22)
                radius: Theme.smallRadius
                color: !root.session.checked ? Theme.background : root.feedback.ok ? Theme.successSoft : Theme.dangerSoft
                border.color: !root.session.checked ? Theme.line : root.feedback.ok ? Theme.success : Theme.danger
                border.width: root.session.checked ? 2 : 1

                Flow {
                    id: answerFlow
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 11 }
                    spacing: 8

                    Repeater {
                        model: tiles.picked

                        delegate: Tile {
                            required property int modelData
                            text: (root.item.tiles ?? [])[modelData] ?? ""
                            onClicked: root.session.unpick(modelData)
                        }
                    }
                }
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8

                Repeater {
                    model: root.item.tiles ?? []

                    delegate: Tile {
                        required property string modelData
                        required property int index
                        text: modelData
                        placeholder: tiles.picked.indexOf(index) >= 0
                        onClicked: root.session.pick(index)
                    }
                }
            }
        }
    }

    Component {
        id: matchView

        RowLayout {
            id: match
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                Repeater {
                    model: root.item.left ?? []

                    delegate: Cell {
                        required property int modelData
                        side: "left"
                        pair: modelData
                        text: (root.item.pairs ?? [])[modelData]?.es ?? ""
                    }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                spacing: 10

                Repeater {
                    model: root.item.right ?? []

                    delegate: Cell {
                        required property int modelData
                        side: "right"
                        pair: modelData
                        text: (root.item.pairs ?? [])[modelData]?.en ?? ""
                    }
                }
            }
        }
    }

    Component {
        id: genderView

        ColumnLayout {
            spacing: 18

            ColumnLayout {
                spacing: 4

                Text {
                    text: root.item.word ?? ""
                    color: Theme.ink
                    font.pixelSize: 34
                    font.weight: Font.Bold
                }
                Muted {
                    text: root.item.en ?? ""
                }
            }
            RowLayout {
                spacing: 14

                Repeater {
                    model: root.item.articles ?? []

                    delegate: AbstractButton {
                        id: articleButton
                        required property string modelData
                        required property int index
                        readonly property bool correct: modelData === (root.item.article ?? "")
                        readonly property bool chosen: root.session.chosenArticle === modelData
                        readonly property color tint: modelData === "el" ? Theme.masculine : Theme.feminine
                        implicitWidth: 120
                        implicitHeight: 64
                        focusPolicy: Qt.NoFocus
                        enabled: !root.session.checked
                        onClicked: root.session.chooseArticle(modelData)

                        contentItem: Text {
                            text: articleButton.modelData
                            color: articleButton.tint
                            font.pixelSize: 24
                            font.weight: Font.Bold
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        background: Rectangle {
                            radius: Theme.radius
                            color: root.session.checked && articleButton.correct ? Theme.successSoft
                                 : root.session.checked && articleButton.chosen ? Theme.dangerSoft
                                 : articleButton.hovered ? Theme.hover : Theme.raised
                            border.color: root.session.checked && articleButton.correct ? Theme.success
                                        : root.session.checked && articleButton.chosen ? Theme.danger
                                        : articleButton.hovered ? articleButton.tint : Theme.edge
                            border.width: 2
                        }

                        Text {
                            anchors { right: parent.right; top: parent.top; margins: 8 }
                            text: articleButton.index + 1
                            color: Theme.faint
                            font.pixelSize: 11
                        }
                    }
                }
            }
        }
    }

    Component {
        id: cardView

        ColumnLayout {
            id: card
            readonly property bool reverse: root.item.reverse ?? false
            spacing: 16

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: cardColumn.implicitHeight + 56
                radius: Theme.radius
                color: Theme.raised
                border.color: Theme.line

                ColumnLayout {
                    id: cardColumn
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 28 }
                    spacing: 10

                    Muted {
                        horizontalAlignment: Text.AlignHCenter
                        text: card.reverse ? "How do you say it in Spanish?" : "What does it mean?"
                    }
                    // Front: Spanish, or English for strong cards.
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        visible: !card.reverse || root.session.revealed
                        spacing: 12

                        SpanishWord {
                            spanish: root.item.es ?? ""
                            gender: root.item.gender ?? ""
                            font.pixelSize: 32
                            font.weight: Font.Bold
                        }
                        SpeakButton {
                            spanish: root.item.es ?? ""
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: card.reverse || root.session.revealed
                        horizontalAlignment: Text.AlignHCenter
                        text: root.item.en ?? ""
                        color: card.reverse && !root.session.revealed ? Theme.ink : Theme.muted
                        font.pixelSize: card.reverse && !root.session.revealed ? 30 : 20
                        font.weight: card.reverse && !root.session.revealed ? Font.Bold : Font.Normal
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: root.session.revealed && text !== ""
                        horizontalAlignment: Text.AlignHCenter
                        text: root.item.example ?? ""
                        color: Theme.ink
                        font.pixelSize: 16
                        font.italic: true
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                }
            }
            PrimaryButton {
                Layout.alignment: Qt.AlignHCenter
                visible: !root.session.revealed
                text: "Show answer  (Space)"
                focusPolicy: Qt.NoFocus
                onClicked: root.session.reveal()
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                visible: root.session.revealed
                spacing: 12

                Repeater {
                    model: [
                        { grade: 0, label: "Again", detail: "Later today" },
                        { grade: 2, label: "Good", detail: card.days(root.item.goodDays ?? 1) },
                        { grade: 3, label: "Easy", detail: card.days(root.item.easyDays ?? 1) }
                    ]

                    delegate: AbstractButton {
                        id: gradeButton
                        required property var modelData
                        required property int index
                        implicitWidth: 132
                        implicitHeight: 58
                        focusPolicy: Qt.NoFocus
                        onClicked: root.session.grade(modelData.grade)

                        contentItem: ColumnLayout {
                            spacing: 1
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: (gradeButton.index + 1) + "  " + gradeButton.modelData.label
                                color: gradeButton.modelData.grade === 0 ? Theme.danger : Theme.ink
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                            }
                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: gradeButton.modelData.detail
                                color: Theme.muted
                                font.pixelSize: 12
                            }
                        }
                        background: Rectangle {
                            radius: Theme.smallRadius
                            color: gradeButton.hovered ? Theme.hover : Theme.raised
                            border.color: gradeButton.hovered ? Theme.accent : Theme.edge
                            border.width: 1.5
                        }
                    }
                }
            }

            function days(n) {
                return n <= 1 ? "Tomorrow" : "In " + n + " days"
            }
        }
    }

    Component {
        id: speakView

        ColumnLayout {
            spacing: 16

            Muted {
                text: "Listen, then repeat it out loud two or three times. Copy the melody, not just the words."
            }
            RowLayout {
                spacing: 12

                SpeakButton {
                    spanish: root.item.es ?? ""
                    diameter: 48
                }
                SpeakButton {
                    spanish: root.item.es ?? ""
                    slow: true
                    diameter: 40
                }
                SpanishWord {
                    Layout.fillWidth: true
                    spanish: root.item.es ?? ""
                    font.pixelSize: 26
                }
            }
            Muted {
                visible: text !== ""
                text: root.item.en ?? ""
            }
            PrimaryButton {
                Layout.topMargin: 4
                text: "I said it out loud  (Enter)"
                focusPolicy: Qt.NoFocus
                onClicked: root.session.selfRate(true)
            }
        }
    }

    Component {
        id: describeView

        ColumnLayout {
            spacing: 14

            Muted {
                text: "You don't know the word for this. Describe it in Spanish, out loud, without English:"
            }
            Prompt {
                text: root.item.en ?? ""
                font.pixelSize: 28
            }
            Flow {
                Layout.fillWidth: true
                spacing: 8

                Repeater {
                    model: root.item.frames ?? []

                    delegate: Rectangle {
                        required property string modelData
                        implicitWidth: frameText.implicitWidth + 22
                        implicitHeight: 32
                        radius: 16
                        color: Theme.successSoft
                        border.color: Theme.goodLine

                        Text {
                            id: frameText
                            anchors.centerIn: parent
                            text: parent.modelData
                            color: Theme.goodInk
                            font.pixelSize: 14
                        }
                    }
                }
            }
            PrimaryButton {
                visible: !root.session.revealed
                text: "Show a model answer  (Space)"
                secondary: true
                focusPolicy: Qt.NoFocus
                onClicked: root.session.reveal()
            }
            Rectangle {
                visible: root.session.revealed
                Layout.fillWidth: true
                implicitHeight: modelColumn.implicitHeight + 28
                radius: Theme.smallRadius
                color: Theme.raised
                border.color: Theme.line

                ColumnLayout {
                    id: modelColumn
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        Text {
                            Layout.fillWidth: true
                            text: root.item.model ?? ""
                            color: Theme.ink
                            font.pixelSize: 17
                            font.italic: true
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                        SpeakButton {
                            spanish: root.item.model ?? ""
                            diameter: 32
                        }
                    }
                    Muted {
                        text: "The word: " + (root.item.es ?? "")
                    }
                }
            }
            RowLayout {
                visible: root.session.revealed
                spacing: 12

                PrimaryButton {
                    text: "1  I managed"
                    focusPolicy: Qt.NoFocus
                    onClicked: root.session.selfRate(true)
                }
                PrimaryButton {
                    text: "2  Not yet"
                    secondary: true
                    focusPolicy: Qt.NoFocus
                    onClicked: root.session.selfRate(false)
                }
            }
        }
    }
}
