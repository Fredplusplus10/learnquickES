import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts

// Settings: day or night, the learner's own voice for everything outside the dialogues, how fast the voice
// speaks at normal and slow pace, and the backup file. Every change is saved as it is made.
ScrollPage {
    id: page
    maxWidth: 1240

    required property LearnerController learner

    readonly property string sample: "Hola, ¿qué tal? Hoy vamos a practicar un poco de español."
    // The backup picked to restore, waiting for the learner to confirm; "" when none.
    property url restoreFile: ""
    property string dataMessage: ""
    property bool dataFailed: false

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
        page.dataFailed = problem !== ""
        page.dataMessage = problem !== "" ? problem : done
    }

    // A pace as a speed, the way voice settings usually say it: ×1.2 longer is 0.83× speed.
    function speedText(pace) {
        return (1 / pace).toFixed(2) + "×"
    }
    function paceText(pace) {
        return "About " + Preferences.syllablesPerSecond(pace).toFixed(1) + " syllables a second  ·  "
               + page.speedText(pace) + " speed"
    }

    // One of the three looks, with a small picture of the app in it.
    component ThemeOption: AbstractButton {
        id: option

        property string value
        property string label
        property string iconName
        property bool previewDark
        property bool previewSplit: false
        readonly property bool current: Preferences.theme === value

        Layout.fillWidth: true
        Layout.preferredWidth: 1
        implicitHeight: optionColumn.implicitHeight + 24
        focusPolicy: Qt.StrongFocus
        onClicked: Preferences.setTheme(value)

        background: Rectangle {
            radius: Theme.smallRadius
            color: option.current ? Theme.accentSoft : option.hovered ? Theme.hover : Theme.raised
            border.color: option.current || option.visualFocus ? Theme.accent : Theme.edge
            border.width: option.current || option.visualFocus ? 2 : 1
        }
        contentItem: Item {}

        ColumnLayout {
            id: optionColumn
            anchors { left: parent.left; right: parent.right; top: parent.top; margins: 12 }
            spacing: 10

            // The app in miniature: sidebar, a card and a button, in day or night colours.
            Item {
                Layout.fillWidth: true
                implicitHeight: 74

                Repeater {
                    model: option.previewSplit ? [false, true] : [option.previewDark]

                    delegate: Item {
                        id: half
                        required property bool modelData
                        required property int index
                        readonly property int count: option.previewSplit ? 2 : 1
                        x: index * parent.width / count
                        width: parent.width / count
                        height: parent.height
                        clip: true

                        Rectangle {
                            x: -half.x
                            width: half.parent.width
                            height: half.height
                            radius: 8
                            color: half.modelData ? "#1b1512" : "#faf7f4"
                            border.color: half.modelData ? "#3a2f29" : "#e9ded7"

                            Rectangle {
                                x: 1
                                y: 1
                                width: parent.width * 0.28
                                height: parent.height - 2
                                radius: 7
                                color: half.modelData ? "#201916" : "#f6f0ed"
                            }
                            Rectangle {
                                x: parent.width * 0.36
                                y: 12
                                width: parent.width * 0.56
                                height: parent.height - 24
                                radius: 6
                                color: half.modelData ? "#241d19" : "#fffdfb"
                                border.color: half.modelData ? "#3a2f29" : "#e9ded7"

                                Rectangle {
                                    x: 8
                                    y: 9
                                    width: parent.width * 0.6
                                    height: 5
                                    radius: 2.5
                                    color: half.modelData ? "#f4ebe6" : "#36231e"
                                }
                                Rectangle {
                                    x: 8
                                    y: 20
                                    width: parent.width * 0.4
                                    height: 4
                                    radius: 2
                                    color: half.modelData ? "#7d6f68" : "#b9aea8"
                                }
                                Rectangle {
                                    anchors { right: parent.right; bottom: parent.bottom; margins: 8 }
                                    width: 22
                                    height: 9
                                    radius: 4.5
                                    color: half.modelData ? "#f07a3a" : "#ed712f"
                                }
                            }
                        }
                    }
                }
            }
            RowLayout {
                spacing: 8

                Icon {
                    name: option.iconName
                    size: 18
                    color: option.current ? Theme.accentStrong : Theme.muted
                }
                Text {
                    text: option.label
                    color: option.current ? Theme.accentStrong : Theme.ink
                    font.pixelSize: 15
                    font.weight: option.current ? Font.DemiBold : Font.Normal
                }
            }
        }
    }

    // A pace: what it is, how fast that is, a button to hear it and the slider.
    component PaceRow: ColumnLayout {
        id: row

        property string label
        property string detail
        property bool slow
        property real value
        property real from
        property real to
        property real recommended
        signal moved(real value)

        Layout.fillWidth: true
        spacing: 6

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Icon {
                name: row.slow ? "turtle" : "speaker"
                size: 20
                color: Theme.accentStrong
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 1

                Text {
                    text: row.label
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    text: row.detail
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
            SpeakButton {
                spanish: page.sample
                slow: row.slow
                voice: Preferences.voice
                diameter: 34
                enabled: Speech.available
            }
        }
        Slider {
            id: slider
            Layout.fillWidth: true
            from: row.from
            to: row.to
            stepSize: 0.05
            snapMode: Slider.SnapAlways
            value: row.value
            enabled: Speech.available
            onMoved: row.moved(Math.round(value * 20) / 20)

            background: Item {
                x: slider.leftPadding
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 200
                implicitHeight: 6
                width: slider.availableWidth
                height: implicitHeight

                Rectangle {
                    anchors.fill: parent
                    radius: 3
                    color: Theme.line
                }
                Rectangle {
                    width: slider.visualPosition * parent.width
                    height: parent.height
                    radius: 3
                    color: slider.enabled ? Theme.accent : Theme.faint
                }
                // Where the recommended pace sits.
                Rectangle {
                    x: (row.recommended - row.from) / (row.to - row.from) * parent.width - width / 2
                    y: -4
                    width: 2
                    height: parent.height + 8
                    radius: 1
                    color: Theme.muted
                }
            }
            handle: Rectangle {
                x: slider.leftPadding + slider.visualPosition * (slider.availableWidth - width)
                y: slider.topPadding + slider.availableHeight / 2 - height / 2
                implicitWidth: 22
                implicitHeight: 22
                radius: 11
                color: Theme.raised
                border.color: slider.enabled ? Theme.accent : Theme.faint
                border.width: slider.visualFocus ? 3 : 2
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Faster"
                color: Theme.muted
                font.pixelSize: 12
            }
            Text {
                Layout.fillWidth: true
                text: page.paceText(row.value)
                color: Theme.ink
                font.pixelSize: 13
                font.weight: Font.DemiBold
                horizontalAlignment: Text.AlignHCenter
                elide: Text.ElideRight
            }
            Text {
                text: "Slower"
                color: Theme.muted
                font.pixelSize: 12
            }
        }
    }

    PageHero {
        overline: "Your app"
        title: "Settings"
        subtitle: "How LearnQuick looks and sounds, and your backup. Every change is saved as you make it."
    }

    Card {
        Layout.fillWidth: true
        title: "Appearance"

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ThemeOption {
                value: "light"
                label: "Day"
                iconName: "sun"
                previewDark: false
            }
            ThemeOption {
                value: "dark"
                label: "Night"
                iconName: "moon"
                previewDark: true
            }
            ThemeOption {
                value: "system"
                label: "Like my computer"
                iconName: "monitor"
                previewSplit: true
            }
        }
    }

    Card {
        id: voiceCard
        Layout.fillWidth: true
        title: "Your narration voice"
        subtitle: "Words, examples and exercises"
        icon: "waveform"

        // The voice a new learner starts with; the one marked Recommended.
        readonly property var recommended: Preferences.voices.find(v => v.style === Preferences.defaultVoice) ?? null

        Text {
            Layout.fillWidth: true
            text: "This voice reads everything outside the dialogues. Dialogue characters keep their own cast, so you "
                  + "still hear a variety of voices and always know who is speaking."
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }

        // Why this one: the recognition test behind the default, not a judgement of taste.
        Rectangle {
            Layout.fillWidth: true
            visible: voiceCard.recommended !== null
            implicitHeight: recommendationRow.implicitHeight + 24
            radius: Theme.smallRadius
            color: Theme.accentSoft

            RowLayout {
                id: recommendationRow
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 14 }
                spacing: 12

                Icon {
                    Layout.alignment: Qt.AlignTop
                    name: "award"
                    size: 20
                    color: Theme.accent
                }
                Text {
                    Layout.fillWidth: true
                    text: "<b>Our recommendation: " + (voiceCard.recommended?.name ?? "") + ".</b> It is the voice "
                          + "you start with: when a speech recogniser transcribed the course's sentences, it misheard "
                          + "the fewest words with this voice."
                    textFormat: Text.StyledText
                    color: Theme.ink
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
        }

        GridLayout {
            Layout.fillWidth: true
            columns: page.width > 760 ? 2 : 1
            rowSpacing: 10
            columnSpacing: 10

            Repeater {
                model: Preferences.voices

                delegate: AbstractButton {
                    id: voiceTile
                    required property var modelData
                    readonly property bool current: Preferences.voice === modelData.style
                    readonly property bool isRecommended: modelData.style === Preferences.defaultVoice

                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    implicitHeight: 68
                    enabled: Speech.available
                    focusPolicy: Qt.StrongFocus
                    checkable: false
                    onClicked: Preferences.setVoice(modelData.style)
                    Keys.onReturnPressed: clicked()
                    Keys.onEnterPressed: clicked()
                    Accessible.role: Accessible.RadioButton
                    Accessible.checked: current
                    Accessible.name: modelData.name + ", " + modelData.detail + (isRecommended ? ", recommended" : "")

                    background: Rectangle {
                        radius: Theme.smallRadius
                        color: voiceTile.current ? Theme.accentSoft : voiceTile.hovered ? Theme.hover : Theme.raised
                        border.color: voiceTile.current || voiceTile.visualFocus ? Theme.accent : Theme.edge
                        border.width: voiceTile.current || voiceTile.visualFocus ? 2 : 1
                    }
                    contentItem: RowLayout {
                        spacing: 12

                        // The radio mark.
                        Rectangle {
                            Layout.leftMargin: 14
                            implicitWidth: 22
                            implicitHeight: 22
                            radius: 11
                            color: "transparent"
                            border.color: voiceTile.current ? Theme.accent : Theme.faint
                            border.width: 2

                            Rectangle {
                                anchors.centerIn: parent
                                visible: voiceTile.current
                                width: 12
                                height: 12
                                radius: 6
                                color: Theme.accent
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 1

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 8

                                Text {
                                    text: voiceTile.modelData.name
                                    color: Theme.ink
                                    font.pixelSize: 16
                                    font.weight: Font.DemiBold
                                }
                                Rectangle {
                                    visible: voiceTile.isRecommended
                                    implicitWidth: recommendedText.implicitWidth + 14
                                    implicitHeight: 20
                                    radius: 10
                                    color: Theme.successSoft

                                    Text {
                                        id: recommendedText
                                        anchors.centerIn: parent
                                        text: "Recommended"
                                        color: Theme.goodInk
                                        font.pixelSize: 11
                                        font.weight: Font.DemiBold
                                    }
                                }
                                Item { Layout.fillWidth: true }
                            }
                            Text {
                                Layout.fillWidth: true
                                text: voiceTile.modelData.detail
                                color: Theme.muted
                                font.pixelSize: 13
                                elide: Text.ElideRight
                            }
                        }
                        // Listening does not change the choice.
                        AbstractButton {
                            id: previewButton
                            Layout.rightMargin: 12
                            implicitWidth: 34
                            implicitHeight: 34
                            enabled: Speech.available
                            focusPolicy: Qt.StrongFocus
                            onClicked: Speech.say(page.sample, false, voiceTile.modelData.style)
                            Keys.onReturnPressed: clicked()
                            Keys.onEnterPressed: clicked()
                            Accessible.name: "Listen to " + voiceTile.modelData.name
                            ToolTip.visible: hovered || visualFocus
                            ToolTip.text: "Listen to " + voiceTile.modelData.name
                            ToolTip.delay: 400

                            background: Rectangle {
                                radius: 17
                                // On the chosen tile, which is already accentSoft, the button stands out in the panel colour.
                                color: previewButton.hovered ? Theme.accentLine : voiceTile.current ? Theme.panel : Theme.accentSoft
                                border.color: previewButton.visualFocus || voiceTile.current ? Theme.accent : "transparent"
                                border.width: previewButton.visualFocus ? 2 : 1
                            }
                            contentItem: Item {
                                Icon {
                                    anchors.centerIn: parent
                                    name: "play"
                                    size: 15
                                    color: Theme.accentStrong
                                }
                            }
                        }
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: Speech.available && Speech.voiceName.indexOf("built in") < 0
            text: "Your computer's own voice is speaking, because the voice built into LearnQuick could not start. "
                  + "These choices apply when it can."
            color: Theme.danger
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Speaking speed"
        subtitle: "For the voice built into LearnQuick"

        PaceRow {
            label: "Normal"
            detail: "Everything the app reads to you."
            slow: false
            value: Preferences.pace
            from: Preferences.minPace
            to: Preferences.maxPace
            recommended: Preferences.defaultPace
            onMoved: value => Preferences.setPace(value)
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 4
            Layout.bottomMargin: 4
            implicitHeight: 1
            color: Theme.line
        }
        PaceRow {
            label: "Slow"
            detail: "The turtle button, to listen again. Always at least a little slower than normal."
            slow: true
            value: Preferences.slowPace
            from: Preferences.minSlowPace
            to: Preferences.maxSlowPace
            recommended: Preferences.defaultSlowPace
            onMoved: value => Preferences.setSlowPace(value)
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 6
            text: "Every voice speaks at the same pace. At 1× it is about "
                  + Preferences.syllablesPerSecond(1).toFixed(1) + " syllables a second, the voices' own pace and "
                  + "quick for a learner. The mark on each slider is the recommended speed, as usually advised for "
                  + "beginners: " + page.speedText(Preferences.defaultPace) + " for normal and "
                  + page.speedText(Preferences.defaultSlowPace) + " for slow, which also leaves a longer pause "
                  + "between sentences. The voice is made slower by drawing out its sounds before they are turned "
                  + "into audio, so the pitch stays the same."
            color: Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
        PrimaryButton {
            Layout.alignment: Qt.AlignRight
            secondary: true
            enabled: !Preferences.recommendedVoice
            opacity: enabled ? 1 : 0.5
            text: "Back to the recommended voice and speeds"
            onClicked: Preferences.resetVoice()
        }
    }

    FileDialog {
        id: saveDialog
        title: "Save a backup of your progress"
        fileMode: FileDialog.SaveFile
        nameFilters: ["LearnQuick backups (*.json)"]
        defaultSuffix: "json"
        onAccepted: page.showResult(page.learner.exportBackup(saveDialog.selectedFile),
                                    "Saved as " + page.fileName(saveDialog.selectedFile)
                                    + ". Keep it somewhere safe, such as a cloud folder.")
    }
    FileDialog {
        id: openDialog
        title: "Restore a LearnQuick backup"
        fileMode: FileDialog.OpenFile
        nameFilters: ["LearnQuick backups (*.json)", "All files (*)"]
        onAccepted: {
            page.dataMessage = ""
            page.restoreFile = openDialog.selectedFile
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Your data"
        subtitle: "Stays on this computer"

        Text {
            Layout.fillWidth: true
            text: "Your progress lives only on this computer. A backup is one file with all of it: your plan, "
                  + "lessons, review cards, streak, listening log and reflections. The web version of LearnQuick "
                  + "opens the same file, so you can move between the two."
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
        Flow {
            Layout.fillWidth: true
            spacing: 12

            PrimaryButton {
                text: "Save a backup…"
                onClicked: {
                    const suggestion = page.learner.backupSuggestion()
                    saveDialog.currentFolder = page.folderOf(suggestion)
                    saveDialog.selectedFile = suggestion
                    saveDialog.open()
                }
            }
            PrimaryButton {
                secondary: true
                enabled: !page.learner.demo
                opacity: enabled ? 1 : 0.5
                text: "Restore from a backup…"
                onClicked: openDialog.open()
            }
            PrimaryButton {
                secondary: true
                text: "Your certificates"
                onClicked: page.openRoute("certificates")
            }
        }
        Text {
            Layout.fillWidth: true
            visible: page.learner.demo
            text: "This is the sample learner, so restoring is turned off. Saving a backup of it works."
            color: Theme.faint
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }

        // Restoring replaces everything, so it waits for a clear yes.
        Rectangle {
            Layout.fillWidth: true
            visible: String(page.restoreFile) !== ""
            implicitHeight: confirmColumn.implicitHeight + 32
            radius: Theme.smallRadius
            color: Theme.dangerSoft
            border.color: Theme.danger

            ColumnLayout {
                id: confirmColumn
                anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 }
                spacing: 12

                Text {
                    Layout.fillWidth: true
                    text: "Replace your progress with " + page.fileName(page.restoreFile) + "?"
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                Text {
                    Layout.fillWidth: true
                    text: "Everything you have now is replaced by what the backup holds, and this cannot be undone. "
                          + "If you might want today's progress back, save a backup of it first."
                    color: Theme.ink
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                }
                Flow {
                    Layout.fillWidth: true
                    spacing: 12

                    PrimaryButton {
                        text: "Replace my progress"
                        onClicked: {
                            const file = page.restoreFile
                            page.restoreFile = ""
                            page.showResult(page.learner.importBackup(file),
                                            "Restored from " + page.fileName(file) + ". Welcome back.")
                        }
                    }
                    PrimaryButton {
                        secondary: true
                        text: "Cancel"
                        onClicked: page.restoreFile = ""
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: page.dataMessage !== ""
            text: page.dataMessage
            color: page.dataFailed ? Theme.danger : Theme.success
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
        }
    }

    Text {
        Layout.fillWidth: true
        visible: Preferences.error !== ""
        text: Preferences.error
        color: Theme.danger
        font.pixelSize: 14
        wrapMode: Text.WordWrap
        textFormat: Text.PlainText
    }
}
