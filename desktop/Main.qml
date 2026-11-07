import QtQuick
import QtQuick.Controls.Basic

// First run shows the welcome flow; once a profile exists, the app shell with Today first.
ApplicationWindow {
    id: window

    required property ProfileController profile
    required property TodayModel today
    required property LearnerController learner
    required property SessionModel session
    property string initialRoute: "today"
    property int welcomeStep: 0

    width: 1100
    height: 720
    minimumWidth: 900
    minimumHeight: 600
    visible: true
    title: "LearnQuick"
    color: Theme.background
    // For what the screens do not colour themselves: tooltips, scroll bars, selections, placeholders.
    palette {
        window: Theme.background
        windowText: Theme.ink
        base: Theme.raised
        alternateBase: Theme.panel
        text: Theme.ink
        button: Theme.panel
        buttonText: Theme.ink
        placeholderText: Theme.faint
        highlight: Theme.accent
        highlightedText: Theme.inkOnAccent
        toolTipBase: Theme.toast
        toolTipText: Theme.inkOnAccent
        mid: Theme.edge
        dark: Theme.faint
        light: Theme.panel
    }

    Loader {
        anchors.fill: parent
        focus: true
        sourceComponent: window.profile.exists ? shell : welcome
    }

    Component {
        id: welcome

        Welcome {
            profile: window.profile
            step: window.welcomeStep
        }
    }

    Component {
        id: shell

        Shell {
            focus: true
            profile: window.profile
            today: window.today
            learner: window.learner
            session: window.session
            route: window.initialRoute
        }
    }

    // The course ships inside the app and progress lives in a local file: this only shows when one is broken.
    Rectangle {
        visible: window.learner.error !== ""
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom; margins: 16 }
        implicitHeight: errorText.implicitHeight + 24
        radius: Theme.smallRadius
        color: Theme.accentSoft
        border.color: Theme.accent

        Text {
            id: errorText
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 14 }
            text: window.learner.error
            textFormat: Text.PlainText
            color: Theme.accentStrong
            wrapMode: Text.WordWrap
        }
    }
}
