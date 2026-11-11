import QtQuick
import QtQuick.Layouts

// Sends the learner to the downloads in their own browser. The app itself makes no network request.
Card {
    id: updates

    property string version
    property string releaseUrl  // empty until lq/release.h names the public repository

    title: "Updates"
    icon: "download"
    subtitle: "Version " + version + " · Check every 15 days for updates"

    Text {
        Layout.fillWidth: true
        text: "LearnQuick never connects to the internet, so it cannot tell you about a new version. About every "
              + "15 days, press the button: it opens the developer's GitHub page in your browser. If the version "
              + "there is newer than " + version + ", download it. Your progress carries over when you update."
        color: Theme.muted
        font.pixelSize: 14
        wrapMode: Text.WordWrap
    }
    Flow {
        Layout.fillWidth: true
        spacing: 12

        PrimaryButton {
            objectName: "downloadLatest"
            enabled: updates.releaseUrl !== ""
            text: "Download latest version"
            onClicked: Qt.openUrlExternally(updates.releaseUrl)
        }
    }
    Text {
        Layout.fillWidth: true
        visible: updates.releaseUrl === ""
        text: "Downloads are not available in this build yet."
        color: Theme.faint
        font.pixelSize: 13
        wrapMode: Text.WordWrap
    }
}
