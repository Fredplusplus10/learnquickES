import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts

// The A1, A2 and B1 Certificates of Readiness: LearnQuick's own learning records, never official CEFR or DELE
// qualifications. Each is issued after that level's assessment, with the name chosen on the welcome screen.
ScrollPage {
    id: page

    maxWidth: 1240

    // certificates/B1 opens that certificate straight away, when it has been earned.
    property string initialLevel: ""

    property string message: ""
    property bool failed: false
    property string viewing: ""  // the level open in the viewer
    readonly property var viewed: {
        for (const entry of Certificates.levels) {
            if (entry.level === page.viewing)
                return entry
        }
        return null
    }

    function fileName(url) {
        const path = decodeURIComponent(String(url))
        return path.substring(path.lastIndexOf("/") + 1)
    }
    function folderOf(url) {
        const path = String(url)
        return path.substring(0, path.lastIndexOf("/"))
    }
    // problem: what the controller returned, "" when it worked.
    function showResult(problem, done) {
        page.failed = problem !== ""
        page.message = problem !== "" ? problem : done
    }
    function save(level, kind) {
        const dialog = kind === "pdf" ? pdfDialog : svgDialog
        const suggestion = Certificates.suggestedFile(level, kind)
        dialog.level = level
        dialog.currentFolder = page.folderOf(suggestion)
        dialog.selectedFile = suggestion
        dialog.open()
    }
    function view(level) {
        page.viewing = level
        viewer.open()
    }

    Connections {
        target: Certificates
        function onIssued(level) { page.view(level) }
    }

    Component.onCompleted: {
        for (const entry of Certificates.levels) {
            if (entry.level === initialLevel && entry.earned)
                view(initialLevel)
        }
    }

    Breadcrumb {
        backLabel: "Progress"
        onBack: page.openRoute("progress")
    }

    PageHero {
        overline: "Achievements"
        title: "Certificates of Readiness"
        subtitle: "A1, A2 and B1 records from LearnQuick, each awarded after that level's own assessment."
    }

    // What these certificates are, and what they are not.
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: noticeRow.implicitHeight + 28
        radius: Theme.smallRadius
        color: Theme.blueSoft
        border.color: Theme.dark ? Theme.edge : "#cfe0f1"

        RowLayout {
            id: noticeRow
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
            spacing: 12

            Icon {
                Layout.alignment: Qt.AlignTop
                name: "shield"
                color: Theme.blue
                size: 22
            }
            Text {
                Layout.fillWidth: true
                text: Certificates.disclaimer
                color: Theme.ink
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
        }
    }

    Card {
        Layout.fillWidth: true
        visible: Certificates.stored
        title: "How a certificate is earned"
        icon: "target"

        Text {
            Layout.fillWidth: true
            text: "Each level's assessment opens once its lessons are done: phase 1 for A1, phases 1 and 2 for A2, "
                  + "all five for B1. It has reading, listening, and grammar and vocabulary, all marked by LearnQuick, "
                  + "and passes with 70% overall and at least 60% in each part. You can take it once a day, and "
                  + "passing it issues the certificate. Days studied, lessons and checkpoints never earn one on "
                  + "their own."
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        visible: Certificates.learnerName !== ""
        spacing: 2

        Text {
            Layout.fillWidth: true
            text: "Your certificates will carry the name you chose when you started:"
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            text: Certificates.learnerName
            textFormat: Text.PlainText
            color: Theme.ink
            font.pixelSize: 18
            font.weight: Font.DemiBold
            elide: Text.ElideRight
        }
    }

    GridLayout {
        Layout.fillWidth: true
        // Three in a row whenever each gets about 220 px; otherwise one below the other.
        columns: Math.min(page.width - 64, page.maxWidth) >= 680 ? 3 : 1
        rowSpacing: 16
        columnSpacing: 16

        Repeater {
            model: Certificates.levels

            delegate: Rectangle {
                id: levelCard
                required property var modelData
                readonly property bool earned: modelData.earned
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                Layout.fillHeight: true
                implicitHeight: levelColumn.implicitHeight + 44
                radius: Theme.radius
                color: earned ? Theme.accentSoft : Theme.panel
                border.color: earned ? Theme.accentLine : Theme.line
                Accessible.role: Accessible.Grouping
                Accessible.name: "Spanish " + modelData.level + (earned ? ", earned" : ", locked")

                ColumnLayout {
                    id: levelColumn
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 22 }
                    spacing: 8

                    Item {
                        Layout.alignment: Qt.AlignHCenter
                        implicitWidth: 104
                        implicitHeight: 116

                        Image {
                            anchors.fill: parent
                            source: "assets/certificates/badges/level_" + levelCard.modelData.level + ".svg"
                            sourceSize.width: 208
                            sourceSize.height: 232
                            fillMode: Image.PreserveAspectFit
                            opacity: levelCard.earned ? 1 : 0.35
                            Accessible.ignored: true
                        }
                        Rectangle {
                            visible: !levelCard.earned
                            anchors { right: parent.right; bottom: parent.bottom; rightMargin: 6; bottomMargin: 10 }
                            width: 34
                            height: 34
                            radius: 17
                            color: Theme.panel
                            border.color: Theme.edge

                            Icon {
                                anchors.centerIn: parent
                                name: "lock"
                                size: 18
                                color: Theme.muted
                            }
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Spanish " + levelCard.modelData.level
                        color: Theme.ink
                        font.pixelSize: 20
                        font.weight: Font.Bold
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Text {
                        Layout.fillWidth: true
                        text: levelCard.modelData.title
                        color: Theme.muted
                        font.pixelSize: 13
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                    }
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        implicitHeight: 1
                        color: levelCard.earned ? Theme.accentLine : Theme.line
                    }

                    // Earned: who it was issued to, when, its ID and what to do with it.
                    Text {
                        Layout.fillWidth: true
                        visible: levelCard.earned
                        text: "Issued to " + levelCard.modelData.username + " on " + levelCard.modelData.issuedOn
                        textFormat: Text.PlainText
                        color: Theme.ink
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth: true
                        visible: levelCard.earned
                        text: "ID " + levelCard.modelData.certificateId
                        color: Theme.muted
                        font.pixelSize: 12
                        wrapMode: Text.WrapAnywhere
                    }
                    PrimaryButton {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        visible: levelCard.earned
                        enabled: String(levelCard.modelData.file) !== ""
                        text: "View certificate"
                        onClicked: page.view(levelCard.modelData.level)
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        visible: levelCard.earned
                        spacing: 8

                        PrimaryButton {
                            Layout.fillWidth: true
                            secondary: true
                            text: "Save SVG…"
                            onClicked: page.save(levelCard.modelData.level, "svg")
                        }
                        PrimaryButton {
                            Layout.fillWidth: true
                            secondary: true
                            text: "Export PDF…"
                            onClicked: page.save(levelCard.modelData.level, "pdf")
                        }
                    }

                    // Not earned: why, in plain words, and the way to earn it.
                    Text {
                        Layout.fillWidth: true
                        visible: !levelCard.earned
                        text: levelCard.modelData.lockedReason
                        color: Theme.muted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }
                    PrimaryButton {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        visible: !levelCard.earned && Certificates.stored
                        enabled: levelCard.modelData.assessmentOpen || levelCard.modelData.ready
                        text: levelCard.modelData.ready ? "Get the certificate"
                                                        : "Take the " + levelCard.modelData.level + " assessment"
                        onClicked: {
                            if (levelCard.modelData.ready)
                                page.showResult(Certificates.issue(levelCard.modelData.level), "")
                            else
                                page.openRoute("assessment/" + levelCard.modelData.level)
                        }
                    }
                }
            }
        }
    }

    Text {
        Layout.fillWidth: true
        visible: page.message !== ""
        text: page.message
        textFormat: Text.PlainText
        color: page.failed ? Theme.danger : Theme.success
        font.pixelSize: 14
        wrapMode: Text.WordWrap
    }

    Text {
        Layout.fillWidth: true
        Layout.topMargin: 6
        text: "Certificates are kept on this computer with your progress. Follow FredPlusPlus10 on GitHub for more."
        color: Theme.muted
        font.pixelSize: 13
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.WordWrap
    }

    FileDialog {
        id: svgDialog
        property string level
        title: "Save your certificate as SVG"
        fileMode: FileDialog.SaveFile
        nameFilters: ["SVG images (*.svg)"]
        defaultSuffix: "svg"
        onAccepted: page.showResult(Certificates.saveSvg(level, selectedFile), "Saved as " + page.fileName(selectedFile) + ".")
    }
    FileDialog {
        id: pdfDialog
        property string level
        title: "Export your certificate as PDF"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PDF documents (*.pdf)"]
        defaultSuffix: "pdf"
        onAccepted: page.showResult(Certificates.exportPdf(level, selectedFile), "Saved as " + page.fileName(selectedFile) + ".")
    }

    // The certificate itself, with the congratulations.
    Popup {
        id: viewer
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(parent.width - 64, 1040)
        height: Math.min(parent.height - 48, viewerColumn.implicitHeight + 48)
        modal: true
        focus: true
        padding: 24
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        Overlay.modal: Rectangle { color: Theme.scrim }
        background: Rectangle {
            radius: Theme.radius
            color: Theme.panel
            border.color: Theme.line
        }

        contentItem: ColumnLayout {
            id: viewerColumn
            spacing: 12

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: "Congratulations, " + (page.viewed ? page.viewed.username : "") + "!"
                        textFormat: Text.PlainText
                        color: Theme.ink
                        font.pixelSize: 24
                        font.weight: Font.Bold
                        elide: Text.ElideRight
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "You have completed the LearnQuick " + page.viewing + " readiness assessment."
                        color: Theme.muted
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }
                }
                AbstractButton {
                    id: closeButton
                    focusPolicy: Qt.StrongFocus
                    Layout.alignment: Qt.AlignTop
                    implicitWidth: 36
                    implicitHeight: 36
                    Accessible.name: "Close"
                    onClicked: viewer.close()
                    contentItem: Item {
                        Icon {
                            anchors.centerIn: parent
                            name: "x"
                            size: 18
                            color: closeButton.hovered ? Theme.ink : Theme.muted
                        }
                    }
                    background: Rectangle {
                        radius: 18
                        color: closeButton.hovered ? Theme.hover : "transparent"
                        border.color: closeButton.visualFocus ? Theme.accent : "transparent"
                    }
                }
            }

            Image {
                id: certificateImage
                Layout.fillWidth: true
                Layout.preferredHeight: width * 1050 / 1485
                Layout.maximumHeight: viewer.parent ? viewer.parent.height - 260 : 600
                source: page.viewed ? page.viewed.file : ""
                sourceSize.width: 2048
                fillMode: Image.PreserveAspectFit
                smooth: true
                asynchronous: true
                cache: false
                Accessible.role: Accessible.Graphic
                Accessible.name: page.viewed ? "Spanish " + page.viewing + " Certificate of Readiness issued to "
                                               + page.viewed.username + " on " + page.viewed.issuedOn : ""
            }

            Text {
                Layout.fillWidth: true
                text: Certificates.disclaimer
                color: Theme.muted
                font.pixelSize: 12
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 10

                PrimaryButton {
                    text: "Save SVG…"
                    onClicked: page.save(page.viewing, "svg")
                }
                PrimaryButton {
                    secondary: true
                    text: "Export PDF…"
                    onClicked: page.save(page.viewing, "pdf")
                }
                PrimaryButton {
                    secondary: true
                    text: "Close"
                    onClicked: viewer.close()
                }
            }
        }
    }
}
