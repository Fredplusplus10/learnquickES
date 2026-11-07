import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// The app once the learner has a profile: the sidebar, the screen for the current route and the toasts.
// Routes: today, lessons, lesson/<unit>[/<step>], practice, practice/<trainer>, vocab, immersion, progress,
// checkpoint/<phase>, mock[/<step>], certificates[/<level>], assessment/<level>, coach, settings.
FocusScope {
    id: shell

    required property ProfileController profile
    required property TodayModel today
    required property LearnerController learner
    required property SessionModel session
    property string route: "today"

    readonly property var parts: route.split("/")
    readonly property string section: parts[0]

    function open(next) {
        session.stop()
        Speech.stop()
        route = next
    }

    // Same component for lesson/3 and lesson/4: rebuild the page so it starts clean.
    onRouteChanged: {
        page.active = false
        page.active = true
    }

    // A sidebar entry: {label, icon, route, sections, phase}. Screens still to come say which phase brings them.
    component NavItem: AbstractButton {
        id: navItem
        property var entry: ({})
        readonly property bool current: (entry.sections ?? []).indexOf(shell.section) >= 0
        Layout.fillWidth: true
        implicitHeight: 40
        enabled: (entry.route ?? "") !== ""
        focusPolicy: Qt.NoFocus
        onClicked: shell.open(entry.route)
        leftPadding: 12
        rightPadding: 10

        background: Rectangle {
            radius: Theme.smallRadius
            color: navItem.current ? Theme.panel : navItem.hovered ? Theme.sidebarHover : "transparent"
            border.color: navItem.current ? Theme.line : "transparent"
        }
        contentItem: RowLayout {
            spacing: 10

            Icon {
                name: navItem.entry.icon ?? ""
                size: 18
                color: navItem.current ? Theme.accentStrong : navItem.enabled ? Theme.muted : Theme.faint
            }
            Text {
                Layout.fillWidth: true
                text: navItem.entry.label ?? ""
                color: navItem.current ? Theme.ink : navItem.enabled ? Theme.muted : Theme.faint
                font.pixelSize: 15
                font.weight: navItem.current ? Font.DemiBold : Font.Normal
            }
            Text {
                visible: (navItem.entry.phase ?? 0) > 0
                text: "Phase " + navItem.entry.phase
                color: Theme.faint
                font.pixelSize: 11
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.preferredWidth: 220
            Layout.fillHeight: true
            color: Theme.sidebar

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 22
                spacing: 6

                RowLayout {
                    Layout.bottomMargin: 24
                    spacing: 10

                    Image {
                        source: "images/learnquick.png"
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34
                        mipmap: true
                    }
                    Text {
                        text: "LearnQuick"
                        color: Theme.ink
                        font.pixelSize: 18
                        font.weight: Font.Bold
                    }
                }

                Repeater {
                    model: [
                        { label: "Today", icon: "home", route: "today", sections: ["today"], phase: 0 },
                        { label: "Lessons", icon: "book", route: "lessons", sections: ["lessons", "lesson", "checkpoint", "mock"], phase: 0 },
                        { label: "Practice", icon: "target", route: "practice", sections: ["practice"], phase: 0 },
                        { label: "Words", icon: "cards", route: "vocab", sections: ["vocab"], phase: 0 },
                        { label: "Immersion", icon: "headphones", route: "immersion", sections: ["immersion"], phase: 0 },
                        { label: "Progress", icon: "chart", route: "progress", sections: ["progress", "certificates", "assessment"], phase: 0 },
                        { label: "Coach", icon: "route", route: "coach", sections: ["coach"], phase: 0 }
                    ]

                    delegate: NavItem {
                        required property var modelData
                        entry: modelData
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                NavItem {
                    Layout.bottomMargin: 8
                    entry: ({ label: "Settings", icon: "settings", route: "settings", sections: ["settings"], phase: 0 })
                }

                // Listening exercises need a Spanish system voice; say how to get one.
                Rectangle {
                    visible: !Speech.available
                    Layout.fillWidth: true
                    implicitHeight: voiceText.implicitHeight + 20
                    radius: Theme.smallRadius
                    color: Theme.panel
                    border.color: Theme.line

                    Text {
                        id: voiceText
                        anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 10 }
                        text: Qt.platform.os === "osx"
                              ? "Audio is off: no Spanish voice. Add one in System Settings → Accessibility → Spoken Content."
                              : "Audio is off: no Spanish voice. Add one in Settings → Time & language → Speech."
                        color: Theme.muted
                        font.pixelSize: 12
                        wrapMode: Text.WordWrap
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: "v" + BuildInfo.appVersion + " · Qt " + BuildInfo.qtVersion
                        color: Theme.faint
                        font.pixelSize: 12
                        elide: Text.ElideRight
                    }
                    // Day or night in one click; Settings also offers following the computer.
                    AbstractButton {
                        id: themeToggle
                        implicitWidth: 32
                        implicitHeight: 32
                        focusPolicy: Qt.NoFocus
                        onClicked: Preferences.setTheme(Theme.dark ? "light" : "dark")
                        ToolTip.visible: hovered
                        ToolTip.delay: 500
                        ToolTip.text: Theme.dark ? "Day mode" : "Night mode"

                        background: Rectangle {
                            radius: height / 2
                            color: themeToggle.hovered ? Theme.sidebarHover : "transparent"
                            border.color: themeToggle.hovered ? Theme.line : "transparent"
                        }
                        contentItem: Item {
                            Icon {
                                anchors.centerIn: parent
                                name: Theme.dark ? "sun" : "moon"
                                size: 17
                                color: themeToggle.hovered ? Theme.accentStrong : Theme.muted
                            }
                        }
                    }
                }
            }
        }

        Loader {
            id: page
            Layout.fillWidth: true
            Layout.fillHeight: true
            focus: true
            sourceComponent: {
                switch (shell.section) {
                case "lessons": return lessonsPage
                case "lesson": return lessonPage
                case "practice": return shell.parts.length > 1 ? trainerPage : practicePage
                case "vocab": return vocabularyPage
                case "immersion": return immersionPage
                case "progress": return progressPage
                case "certificates": return certificatesPage
                case "assessment": return assessmentPage
                case "checkpoint": return checkpointPage
                case "mock": return mockPage
                case "coach": return coachPage
                case "settings": return settingsPage
                default: return todayPage
                }
            }
        }
    }

    Component {
        id: todayPage

        TodayPage {
            focus: true
            profile: shell.profile
            today: shell.today
            learner: shell.learner
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: lessonsPage

        LessonsPage {
            focus: true
            learner: shell.learner
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: lessonPage

        LessonPage {
            focus: true
            learner: shell.learner
            session: shell.session
            unitIndex: parseInt(shell.parts[1] ?? "0")
            initialStep: shell.parts[2] ?? "vocab"
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: trainerPage

        TrainerPage {
            focus: true
            learner: shell.learner
            session: shell.session
            trainerId: shell.parts[1] ?? "verbs"
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: checkpointPage

        CheckpointPage {
            focus: true
            learner: shell.learner
            session: shell.session
            phase: parseInt(shell.parts[1] ?? "1")
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: mockPage

        MockExamPage {
            focus: true
            learner: shell.learner
            session: shell.session
            initialStep: shell.parts[1] ?? "intro"
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: coachPage

        CoachPage {
            focus: true
            learner: shell.learner
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: practicePage

        PracticePage {
            focus: true
            learner: shell.learner
            today: shell.today
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: vocabularyPage

        VocabularyPage {
            focus: true
            learner: shell.learner
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: immersionPage

        ImmersionPage {
            focus: true
            learner: shell.learner
            initialKind: shell.parts[1] ?? "podcast"
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: certificatesPage

        CertificatesPage {
            focus: true
            initialLevel: shell.parts[1] ?? ""
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: assessmentPage

        AssessmentPage {
            focus: true
            learner: shell.learner
            session: shell.session
            level: shell.parts[1] ?? "A1"
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: progressPage

        ProgressPage {
            focus: true
            learner: shell.learner
            today: shell.today
            profile: shell.profile
            onOpenRoute: route => shell.open(route)
        }
    }

    Component {
        id: settingsPage

        SettingsPage {
            focus: true
            learner: shell.learner
            onOpenRoute: route => shell.open(route)
        }
    }

    // Celebrations from the engine (streak, day or phase complete, an achievement), one at a time.
    property var toasts: []

    Connections {
        target: shell.learner
        function onCelebrated(title, detail, icon) {
            shell.toasts = shell.toasts.concat([{ title: title, detail: detail, icon: icon }])
            if (!toast.showing)
                toast.showNext()
        }
    }

    Rectangle {
        id: toast

        property bool showing: false
        property string title
        property string detail
        property string icon

        function showNext() {
            if (shell.toasts.length === 0) {
                showing = false
                return
            }
            title = shell.toasts[0].title
            detail = shell.toasts[0].detail
            icon = shell.toasts[0].icon
            shell.toasts = shell.toasts.slice(1)
            showing = true
            toastTimer.restart()
        }

        anchors { horizontalCenter: parent.horizontalCenter; horizontalCenterOffset: 110; bottom: parent.bottom; bottomMargin: showing ? 28 : -height }
        implicitWidth: Math.min(toastRow.implicitWidth + 36, shell.width - 280)
        implicitHeight: toastRow.implicitHeight + 26
        radius: Theme.radius
        color: Theme.toast
        opacity: showing ? 1 : 0
        Behavior on anchors.bottomMargin { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }
        Behavior on opacity { NumberAnimation { duration: 220 } }

        RowLayout {
            id: toastRow
            anchors.centerIn: parent
            width: parent.width - 36
            spacing: 12

            Icon {
                visible: toast.icon !== "streak"
                name: toast.icon !== "" ? toast.icon : "sparkles"
                color: Theme.toastIcon
                size: 22
            }
            StreakIcon {
                visible: toast.icon === "streak"
                emoji: shell.profile.streakEmoji
                size: 26
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Text {
                    text: toast.title
                    color: Theme.inkOnAccent
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: toast.detail
                    color: Theme.toastMuted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
        }

        Timer {
            id: toastTimer
            interval: 3500
            onTriggered: toast.showNext()
        }
    }
}
