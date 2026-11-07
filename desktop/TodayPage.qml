import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The daily dashboard. Every number on it comes from the C++ engine through TodayModel and LearnerController;
// each task, word and quick action opens the screen that runs it.
FocusScope {
    id: page

    required property ProfileController profile
    required property TodayModel today
    required property LearnerController learner
    signal openRoute(string route)

    // The first task still to do gets the "Start" button.
    readonly property int nextTask: {
        const tasks = today.tasks
        for (let i = 0; i < tasks.length; ++i) {
            if (!tasks[i].done)
                return i
        }
        return -1
    }
    readonly property int tasksLeft: {
        let n = 0
        for (const task of today.tasks)
            n += task.done ? 0 : 1
        return n
    }
    readonly property int minutesLeft: {
        let n = 0
        for (const task of today.tasks)
            n += task.done ? 0 : task.minutes
        return n
    }
    // The first few words due in the learner's deck. dueCount changes with the deck, so the list follows it.
    readonly property var dueWords: {
        if (learner.dueCount <= 0)
            return []
        return learner.vocabulary().filter(word => word.state === "due").slice(0, 3)
    }
    readonly property real share: today.totalDays > 0 ? today.doneCount / today.totalDays : 0

    // Three columns when the window has room for them, otherwise the coach joins the middle column.
    readonly property bool wide: content.width >= 1060

    function heroLine() {
        if (today.finished)
            return "Your plan is complete. Keep your Spanish going with review and immersion."
        if (today.finishedToday || tasksLeft === 0)
            return "Today's plan is done. Review a few words or log some listening if you have time."
        return (tasksLeft === 1 ? "1 task left today" : tasksLeft + " tasks left today")
               + ", about " + minutesLeft + " min, plus Spanish listening outside the app."
    }

    Flickable {
        id: scroller
        anchors.fill: parent
        contentWidth: width
        contentHeight: content.implicitHeight + 56
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: content
            x: 32
            y: 26
            width: scroller.width - 64
            spacing: 18

            PageHero {
                overline: page.today.dateText
                title: "¡Hola, " + page.today.learnerName + "!"
                highlight: page.today.totalDays > 0 ? "Day " + page.today.dayNumber + " of " + page.today.totalDays : ""
                detail: page.today.phase > 0 ? page.today.phaseTitle : ""
                subtitle: page.heroLine()

                // The streak with the learner's own icon; clicking it picks another.
                AbstractButton {
                    id: streakPill
                    implicitWidth: pillRow.implicitWidth + 30
                    implicitHeight: 40
                    focusPolicy: Qt.StrongFocus
                    onClicked: streakPicker.open()
                    Accessible.name: page.today.streak + "-day streak. Change your streak icon"
                    ToolTip.visible: hovered
                    ToolTip.text: "Change your streak icon"
                    ToolTip.delay: 500
                    contentItem: Item {
                        RowLayout {
                            id: pillRow
                            anchors.centerIn: parent
                            spacing: 6

                            StreakIcon {
                                emoji: page.profile.streakEmoji
                                lit: page.today.streak > 0
                                size: 22
                            }
                            Text {
                                text: page.today.streak + "-day streak"
                                color: Theme.accentStrong
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                            }
                        }
                    }
                    background: Rectangle {
                        radius: 20
                        color: streakPill.hovered ? Theme.accentHover : Theme.panel
                        border.color: Theme.accent
                        border.width: streakPill.visualFocus ? 2 : 1
                    }
                }
            }

            // The learner's own reason, from the welcome screen.
            Rectangle {
                Layout.fillWidth: true
                implicitHeight: whyRow.implicitHeight + 26
                radius: Theme.smallRadius
                color: Theme.accentSoft
                border.color: Theme.accentLine

                Rectangle {
                    width: 4
                    radius: 2
                    color: Theme.accent
                    anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 10 }
                }
                RowLayout {
                    id: whyRow
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                    anchors.leftMargin: 26
                    anchors.rightMargin: 18
                    spacing: 14

                    Text {
                        Layout.alignment: Qt.AlignTop
                        text: "YOUR WHY"
                        color: Theme.accentStrong
                        font.pixelSize: 12
                        font.weight: Font.Bold
                        font.letterSpacing: 0.8
                        topPadding: 2
                    }
                    Text {
                        Layout.fillWidth: true
                        text: page.today.reason
                        textFormat: Text.PlainText
                        color: Theme.ink
                        font.pixelSize: 15
                        font.italic: true
                        wrapMode: Text.WordWrap
                        maximumLineCount: 2
                        elide: Text.ElideRight
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                // ---- Today's plan -------------------------------------------------------------------------
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 420
                    Layout.alignment: Qt.AlignTop
                    spacing: 18

                    Card {
                        Layout.fillWidth: true
                        title: "Today's plan"
                        icon: "calendar"

                        Text {
                            Layout.fillWidth: true
                            text: page.today.finished ? "Every day of your plan is done."
                                                      : "Complete these tasks to finish the day and move forward."
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                        }
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            visible: page.today.finished
                            source: visible ? "assets/illustrations/empty-plan-done.svg" : ""
                            sourceSize: Qt.size(200, 150)
                            Layout.preferredWidth: 200
                            Layout.preferredHeight: 150
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.topMargin: 4
                            visible: page.today.phase > 0
                            text: ("Phase " + page.today.phase + " · " + page.today.phaseTitle).toUpperCase()
                            color: Theme.muted
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            font.letterSpacing: 0.6
                            elide: Text.ElideRight
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 16

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Text {
                                    Layout.fillWidth: true
                                    text: page.today.dayTitle
                                    color: Theme.ink
                                    font.pixelSize: 24
                                    font.weight: Font.Bold
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: page.today.dayTitleEs
                                    color: Theme.muted
                                    font.pixelSize: 15
                                    font.italic: true
                                    wrapMode: Text.WordWrap
                                }
                            }
                            ColumnLayout {
                                Layout.alignment: Qt.AlignTop
                                spacing: 2

                                Text {
                                    Layout.alignment: Qt.AlignRight
                                    text: page.today.minutes + " min"
                                    color: Theme.ink
                                    font.pixelSize: 22
                                    font.weight: Font.Bold
                                }
                                Text {
                                    Layout.alignment: Qt.AlignRight
                                    text: page.today.dayKind
                                    color: Theme.muted
                                    font.pixelSize: 13
                                }
                            }
                        }

                        // Today's plan is done: the next plan day is already below, for getting ahead.
                        Rectangle {
                            visible: page.today.finishedToday && !page.today.finished
                            Layout.fillWidth: true
                            implicitHeight: aheadText.implicitHeight + 22
                            radius: Theme.smallRadius
                            color: Theme.successSoft
                            border.color: Theme.goodLine

                            Text {
                                id: aheadText
                                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 14 }
                                text: "Today's plan is done. ¡Bien hecho! Want to get ahead? This is day " + page.today.dayNumber + "."
                                color: Theme.goodInk
                                font.pixelSize: 14
                                wrapMode: Text.WordWrap
                            }
                        }

                        Repeater {
                            model: page.today.tasks

                            delegate: AbstractButton {
                                id: task
                                required property var modelData
                                required property int index
                                readonly property bool next: index === page.nextTask
                                Layout.fillWidth: true
                                implicitHeight: Math.max(60, taskText.implicitHeight + 22)
                                enabled: !page.today.finished
                                focusPolicy: Qt.StrongFocus
                                Keys.onReturnPressed: clicked()
                                Keys.onEnterPressed: clicked()
                                onClicked: page.openRoute(task.modelData.route)
                                Accessible.name: task.modelData.name + ", " + task.modelData.minutes + " minutes"
                                                 + (task.modelData.done ? ", done" : "")

                                background: Rectangle {
                                    radius: Theme.smallRadius
                                    color: task.hovered ? Theme.accentSoft : Theme.background
                                    border.color: task.visualFocus ? Theme.accent : task.hovered ? Theme.hoverLine : Theme.line
                                    border.width: task.visualFocus ? 2 : 1

                                    Rectangle {
                                        width: 4
                                        radius: 2
                                        color: Theme.tone(task.modelData.tone)
                                        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; margins: 10 }
                                    }
                                }

                                leftPadding: 26
                                rightPadding: 14

                                contentItem: RowLayout {
                                    spacing: 12

                                    ColumnLayout {
                                        id: taskText
                                        Layout.fillWidth: true
                                        spacing: 1

                                        Text {
                                            Layout.fillWidth: true
                                            text: task.modelData.name
                                            color: Theme.ink
                                            font.pixelSize: 15
                                            font.weight: Font.DemiBold
                                            elide: Text.ElideRight
                                        }
                                        Text {
                                            Layout.fillWidth: true
                                            text: task.modelData.detail
                                            color: Theme.muted
                                            font.pixelSize: 13
                                            elide: Text.ElideRight
                                        }
                                    }
                                    Text {
                                        text: task.modelData.minutes + " min"
                                        color: Theme.muted
                                        font.pixelSize: 13
                                    }
                                    Rectangle {
                                        visible: task.next
                                        implicitWidth: startText.implicitWidth + 26
                                        implicitHeight: 32
                                        radius: 16
                                        color: task.hovered ? Theme.accentPressed : Theme.accent

                                        Text {
                                            id: startText
                                            anchors.centerIn: parent
                                            text: "Start"
                                            color: Theme.inkOnAccent
                                            font.pixelSize: 14
                                            font.weight: Font.DemiBold
                                        }
                                    }
                                    Rectangle {
                                        visible: !task.next
                                        implicitWidth: 24
                                        implicitHeight: 24
                                        radius: 12
                                        color: task.modelData.done ? Theme.success : "transparent"
                                        border.color: task.modelData.done ? Theme.success : Theme.faint
                                        border.width: 2

                                        Icon {
                                            anchors.centerIn: parent
                                            visible: task.modelData.done
                                            name: "check"
                                            color: Theme.inkOnAccent
                                            size: 15
                                        }
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.topMargin: 4
                            implicitHeight: 1
                            color: Theme.line
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 12

                            Icon {
                                Layout.alignment: Qt.AlignTop
                                name: "headphones"
                                color: Theme.muted
                                size: 20
                            }
                            Text {
                                Layout.fillWidth: true
                                text: "Click a task to start it. Also aim for " + page.today.inputMinutes
                                      + " min of Spanish listening today: podcasts, series, music."
                                color: Theme.muted
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                            }
                        }
                    }

                    // Narrow windows: quick actions and words fill the space below the plan.
                    Loader {
                        Layout.fillWidth: true
                        active: !page.wide
                        visible: active
                        sourceComponent: actionsCard
                    }
                    Loader {
                        Layout.fillWidth: true
                        active: !page.wide
                        visible: active
                        sourceComponent: wordsCard
                    }
                }

                // ---- Progress, streak, words and quick actions ---------------------------------------------
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 380
                    Layout.minimumWidth: 300
                    Layout.alignment: Qt.AlignTop
                    spacing: 18

                    // Narrow windows: the coach leads this column instead of having its own.
                    Loader {
                        Layout.fillWidth: true
                        active: !page.wide
                        visible: active
                        sourceComponent: coachCard
                    }

                    Card {
                        Layout.fillWidth: true
                        title: "Your progress"
                        icon: "target"

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 14

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 3

                                Text {
                                    Layout.fillWidth: true
                                    text: "Day " + page.today.dayNumber + " of " + page.today.totalDays
                                    color: Theme.ink
                                    font.pixelSize: 24
                                    font.weight: Font.Bold
                                    elide: Text.ElideRight
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: page.today.doneCount + (page.today.doneCount === 1 ? " day done · " : " days done · ")
                                          + Math.max(0, page.today.totalDays - page.today.doneCount) + " to go"
                                    color: Theme.muted
                                    font.pixelSize: 13
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    visible: page.today.paceText !== ""
                                    text: page.today.paceText
                                    color: page.today.paceDiff < 0 ? Theme.accentStrong : Theme.muted
                                    font.pixelSize: 13
                                    wrapMode: Text.WordWrap
                                }
                                Text {
                                    Layout.fillWidth: true
                                    Layout.topMargin: 4
                                    visible: page.today.targetText !== ""
                                    text: "Projected finish: " + page.today.targetText
                                    color: Theme.ink
                                    font.pixelSize: 13
                                    wrapMode: Text.WordWrap
                                }
                            }
                            ProgressRing {
                                Layout.alignment: Qt.AlignTop
                                value: page.share
                                label: Math.round(page.share * 100) + "%"
                                caption: "Complete"
                            }
                        }
                    }

                    StreakCard {
                        Layout.fillWidth: true
                        today: page.today
                        profile: page.profile
                        onChangeIcon: streakPicker.open()
                    }

                    Loader {
                        Layout.fillWidth: true
                        active: page.wide
                        visible: active
                        sourceComponent: wordsCard
                    }

                    Loader {
                        Layout.fillWidth: true
                        active: page.wide
                        visible: active
                        sourceComponent: actionsCard
                    }
                }

                // ---- Coach, in its own column on wide windows ----------------------------------------------
                Loader {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 330
                    Layout.minimumWidth: 290
                    Layout.maximumWidth: 380
                    Layout.alignment: Qt.AlignTop
                    active: page.wide
                    visible: active
                    sourceComponent: coachCard
                }
            }

            // ---- The plan's phases and the calendar, below the day's work ---------------------------------
            RowLayout {
                Layout.fillWidth: true
                spacing: 18

                Card {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    title: "Your route"
                    subtitle: page.today.doneCount + " of " + page.today.totalDays + " days done"
                    icon: "route"

                    Rectangle {
                        Layout.fillWidth: true
                        implicitHeight: 10
                        radius: 5
                        color: Theme.line

                        Rectangle {
                            width: parent.width * page.share
                            height: parent.height
                            radius: 5
                            color: Theme.accent
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Repeater {
                            model: page.today.phases

                            delegate: ColumnLayout {
                                id: phaseItem
                                required property var modelData
                                readonly property bool current: modelData.state === "current"
                                Layout.fillWidth: true
                                Layout.preferredWidth: 1
                                Layout.alignment: Qt.AlignTop
                                spacing: 6

                                Rectangle {
                                    Layout.fillWidth: true
                                    implicitHeight: 6
                                    radius: 3
                                    color: phaseItem.modelData.state === "done" ? Theme.success
                                         : phaseItem.current ? Theme.accent : Theme.line
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: phaseItem.modelData.n + ". " + phaseItem.modelData.title
                                    color: phaseItem.current ? Theme.ink : Theme.muted
                                    font.pixelSize: 12
                                    font.weight: phaseItem.current ? Font.DemiBold : Font.Normal
                                    wrapMode: Text.WordWrap
                                    maximumLineCount: 2
                                    elide: Text.ElideRight
                                }
                            }
                        }
                    }
                }

                CalendarCard {
                    Layout.preferredWidth: 330
                    Layout.minimumWidth: 290
                    Layout.maximumWidth: 380
                    Layout.alignment: Qt.AlignTop
                    today: page.today
                }
            }
        }
    }

    Component {
        id: coachCard

        CoachCard {
            icon: "lightbulb"
            tips: page.today.coach
            more: "More from your coach"
            onFollow: route => page.openRoute(route)
        }
    }

    Component {
        id: wordsCard

        Card {
            title: "Words to review"
            icon: "book"

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: -6

                Text {
                    Layout.fillWidth: true
                    text: page.learner.dueCount > 0
                          ? (page.learner.dueCount === 1 ? "1 word is due" : page.learner.dueCount + " words are due")
                            + " in your spaced-repetition deck."
                          : page.learner.deckSize > 0
                            ? "Nothing is due today. Your words come back just before you forget them."
                            : "No words yet: they join your deck when you finish a lesson."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                AbstractButton {
                    id: seeAll
                    Layout.alignment: Qt.AlignTop
                    focusPolicy: Qt.StrongFocus
                    onClicked: page.openRoute("vocab")
                    Accessible.name: "See all words"
                    contentItem: RowLayout {
                        spacing: 2
                        Text {
                            text: "See all"
                            color: seeAll.hovered || seeAll.visualFocus ? Theme.accentStrong : Theme.accent
                            font.pixelSize: 13
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

            Repeater {
                model: page.dueWords

                delegate: Rectangle {
                    id: wordRow
                    required property var modelData
                    Layout.fillWidth: true
                    implicitHeight: 40
                    radius: Theme.smallRadius
                    color: Theme.background
                    border.color: Theme.line

                    RowLayout {
                        anchors { fill: parent; leftMargin: 12; rightMargin: 8 }
                        spacing: 6

                        Text {
                            text: wordRow.modelData.es
                            textFormat: Text.PlainText
                            color: Theme.ink
                            font.pixelSize: 14
                            font.weight: Font.DemiBold
                        }
                        SpeakButton {
                            spanish: wordRow.modelData.es
                            diameter: 28
                            Accessible.name: "Listen to " + wordRow.modelData.es
                        }
                        Text {
                            Layout.fillWidth: true
                            text: wordRow.modelData.en
                            textFormat: Text.PlainText
                            color: Theme.muted
                            font.pixelSize: 13
                            horizontalAlignment: Text.AlignRight
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }

    Component {
        id: actionsCard

        Card {
            title: "Quick actions"
            icon: "bolt"

            // One row when there is room, two by two in a column, one under another when that is narrow:
            // an icon, the action and a detail line, wrapping rather than cut short.
            GridLayout {
                id: actions
                Layout.fillWidth: true
                columns: width >= 640 ? 4 : width >= 320 ? 2 : 1
                rowSpacing: 8
                columnSpacing: 8

                Repeater {
                    model: [
                        page.nextTask >= 0
                            ? { icon: "play", label: "Continue today", detail: page.today.tasks[page.nextTask].name,
                                route: page.today.tasks[page.nextTask].route }
                            : { icon: "book", label: "Open lessons", detail: "All 25 lessons", route: "lessons" },
                        { icon: "cards", label: "Flashcards",
                          detail: page.learner.dueCount > 0 ? page.learner.dueCount + " due" : "Nothing due",
                          route: "practice/review" },
                        { icon: "palm", label: "Spanish island", detail: "Think only in Spanish", route: "immersion/island" },
                        { icon: "headphones", label: "Log listening", detail: "Podcasts, series, music", route: "immersion" }
                    ]

                    delegate: AbstractButton {
                        id: action
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true  // a row's tiles share the tallest one's height
                        implicitHeight: Math.max(62, actionRow.implicitHeight + 20)
                        focusPolicy: Qt.StrongFocus
                        onClicked: page.openRoute(action.modelData.route)
                        Accessible.name: action.modelData.label + ", " + action.modelData.detail

                        background: Rectangle {
                            radius: Theme.smallRadius
                            color: action.hovered ? Theme.accentSoft : Theme.background
                            border.color: action.visualFocus ? Theme.accent : action.hovered ? Theme.hoverLine : Theme.line
                            border.width: action.visualFocus ? 2 : 1
                        }
                        contentItem: Item {
                            RowLayout {
                                id: actionRow
                                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter }
                                anchors.leftMargin: 10
                                anchors.rightMargin: 8
                                spacing: 10

                                Rectangle {
                                    implicitWidth: 34
                                    implicitHeight: 34
                                    radius: 17
                                    color: Theme.accentSoft

                                    Icon {
                                        anchors.centerIn: parent
                                        name: action.modelData.icon
                                        color: Theme.accent
                                        size: 18
                                    }
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 1

                                    Text {
                                        Layout.fillWidth: true
                                        text: action.modelData.label
                                        color: Theme.ink
                                        font.pixelSize: 13
                                        font.weight: Font.DemiBold
                                        wrapMode: Text.WordWrap
                                    }
                                    Text {
                                        Layout.fillWidth: true
                                        text: action.modelData.detail
                                        textFormat: Text.PlainText
                                        color: Theme.muted
                                        font.pixelSize: 12
                                        wrapMode: Text.WordWrap
                                        maximumLineCount: 2
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    StreakEmojiPicker {
        id: streakPicker
        profile: page.profile
    }
}
