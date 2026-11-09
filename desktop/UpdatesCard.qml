import QtQuick
import QtQuick.Layouts

// Sends the learner to the downloads in their own browser. The app itself makes no network request.
Card {
    id: updates

    property string version
    property string releaseUrl  // empty until lq/release.h names the public repository

    title: "Updates"
    icon: "download"
    subtitle: "You have version " + version

    Text {
        Layout.fillWidth: true
        text: "New versions of LearnQuick are published on GitHub. The page opens in your browser, and your "
              + "progress carries over when you update."
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
