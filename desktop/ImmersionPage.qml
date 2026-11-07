import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Dialogs
import QtQuick.Layouts

// Spanish outside the app: today's minutes, a log of what you listened to with its history, the listening
// milestones, viewing imported from Netflix and the like, then resources, device missions and language exchanges.
// Logged minutes count as Spanish time and earn their number in XP; imported viewing earns none.
ScrollPage {
    id: page

    required property LearnerController learner
    maxWidth: 1240

    readonly property real columnWidth: Math.min(width - 64, maxWidth)
    // The log and its history on the left, milestones and tips beside them, when there is room.
    readonly property bool wide: columnWidth >= 960
    // The kind picked when the page opens, e.g. "island" from Today's quick actions; unknown kinds keep the default.
    property string initialKind: "podcast"

    // Not `data`: every Item already has a property of that name.
    property var info: learner.immersion()

    Connections {
        target: page.learner
        function onChanged() { page.info = page.learner.immersion() }
    }

    component StepButton: AbstractButton {
        id: stepButton
        implicitWidth: 34
        implicitHeight: 34
        focusPolicy: Qt.NoFocus
        background: Rectangle {
            radius: 17
            color: stepButton.hovered && stepButton.enabled ? Theme.hover : Theme.raised
            border.color: Theme.edge
        }
        contentItem: Text {
            text: stepButton.text
            color: stepButton.enabled ? Theme.ink : Theme.faint
            font.pixelSize: 18
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    // Minutes, by steps of 5, typed or with the buttons.
    component MinutesField: RowLayout {
        id: minutesField
        property int value: 20
        property int from: 1
        property int to: 600
        spacing: 6

        function set(v) {
            value = Math.max(from, Math.min(to, v))
            minutesInput.text = value
        }

        StepButton {
            text: "−"
            enabled: minutesField.value > minutesField.from
            onClicked: minutesField.set(minutesField.value - 5)
        }
        InputField {
            id: minutesInput
            implicitWidth: 64
            horizontalAlignment: Text.AlignHCenter
            text: minutesField.value
            validator: IntValidator { bottom: minutesField.from; top: minutesField.to }
            onTextEdited: if (acceptableInput) minutesField.set(parseInt(text))
            onEditingFinished: text = minutesField.value
        }
        StepButton {
            text: "+"
            enabled: minutesField.value < minutesField.to
            onClicked: minutesField.set(minutesField.value + 5)
        }
        Text {
            text: "min"
            color: Theme.muted
            font.pixelSize: 14
        }
    }

    // A thin bar from 0 to 1.
    component Meter: Rectangle {
        property real value: 0
        Layout.fillWidth: true
        implicitHeight: 8
        radius: 4
        color: Theme.line

        Rectangle {
            width: parent.width * Math.max(0, Math.min(1, parent.value))
            height: parent.height
            radius: 4
            color: Theme.accent
        }
    }

    // A dated line of a log: date, minutes, what it was, and a delete button when `removable`.
    component LogLine: RowLayout {
        id: logLine
        property string date
        property int minutes
        property string detail
        property bool removable: false
        signal remove()
        Layout.fillWidth: true
        spacing: 12

        Text {
            Layout.preferredWidth: 52
            text: logLine.date
            color: Theme.muted
            font.pixelSize: 13
        }
        Text {
            Layout.preferredWidth: 56
            text: logLine.minutes + " min"
            color: Theme.ink
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }
        Text {
            Layout.fillWidth: true
            text: logLine.detail
            color: Theme.ink
            font.pixelSize: 14
            elide: Text.ElideRight
            textFormat: Text.PlainText
        }
        AbstractButton {
            id: removeButton
            visible: logLine.removable
            implicitWidth: 30
            implicitHeight: 30
            focusPolicy: Qt.StrongFocus
            onClicked: logLine.remove()
            ToolTip.visible: hovered
            ToolTip.text: "Delete this entry"
            ToolTip.delay: 400
            background: Rectangle {
                radius: 15
                color: removeButton.hovered ? Theme.dangerSoft : "transparent"
            }
            contentItem: Item {
                Icon {
                    anchors.centerIn: parent
                    name: "trash"
                    size: 16
                    color: removeButton.hovered ? Theme.danger : Theme.faint
                }
            }
        }
    }

    component LinkButton: AbstractButton {
        id: linkButton
        property string url
        implicitHeight: 38
        leftPadding: 16
        rightPadding: 14
        focusPolicy: Qt.StrongFocus
        onClicked: Qt.openUrlExternally(url)
        background: Rectangle {
            radius: height / 2
            color: linkButton.hovered ? Theme.hover : Theme.raised
            border.color: linkButton.visualFocus ? Theme.accent : Theme.edge
        }
        contentItem: RowLayout {
            spacing: 6
            Text {
                text: linkButton.text
                color: Theme.ink
                font.pixelSize: 14
                font.weight: Font.DemiBold
            }
            Icon {
                name: "external"
                size: 15
                color: Theme.muted
            }
        }
    }

    PageHero {
        overline: "Immersion"
        title: "Spanish outside the app"
        subtitle: "The app teaches the system; immersion builds the ear and the habit. Log your listening, switch "
                  + "your devices to Spanish and talk to real people. Every minute counts as Spanish time and earns XP."
    }

    // Today's minutes against the plan's target, before anything else.
    Rectangle {
        Layout.fillWidth: true
        implicitHeight: exposureColumn.implicitHeight + 36
        radius: Theme.radius
        color: Theme.accentSoft
        border.color: Theme.accentLine

        ColumnLayout {
            id: exposureColumn
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 20 }
            spacing: 12

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                Icon {
                    Layout.alignment: Qt.AlignTop
                    name: "headphones"
                    color: Theme.accent
                    size: 24
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        Layout.fillWidth: true
                        text: "Today's Spanish exposure"
                        color: Theme.ink
                        font.pixelSize: 17
                        font.weight: Font.Bold
                        wrapMode: Text.WordWrap
                    }
                    Text {
                        Layout.fillWidth: true
                        text: page.info.target + " min daily target from your plan · listening, the language island "
                              + "and conversations count towards it"
                        color: Theme.muted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }
                }
                Text {
                    text: page.info.todayMinutes + " / " + page.info.target + " min"
                    color: page.info.todayMinutes >= page.info.target ? Theme.goodInk : Theme.ink
                    font.pixelSize: 22
                    font.weight: Font.Bold
                }
            }
            Meter {
                value: page.info.target > 0 ? page.info.todayMinutes / page.info.target : 0
            }
        }
    }

    GridLayout {
        Layout.fillWidth: true
        columns: page.wide ? 2 : 1
        columnSpacing: 20
        rowSpacing: 20

        ColumnLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "Log your Spanish"
                subtitle: "Manual entry"
                icon: "pen"

                Text {
                    text: "What did you practise?"
                    color: Theme.ink
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                Flow {
                    id: kindFlow
                    property string kind: page.info.kinds.some(k => k.id === page.initialKind) ? page.initialKind : "podcast"
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: page.info.kinds

                        delegate: Chip {
                            required property var modelData
                            text: modelData.label
                            checked: kindFlow.kind === modelData.id
                            onClicked: kindFlow.kind = modelData.id
                        }
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: kindFlow.kind === "island"
                    text: "The language island: time with no English at all. If you don't know a word, describe it in Spanish."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.topMargin: 4
                    text: "Duration"
                    color: Theme.ink
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                GridLayout {
                    Layout.fillWidth: true
                    columns: page.columnWidth >= 620 ? 2 : 1
                    columnSpacing: 12
                    rowSpacing: 10

                    MinutesField {
                        id: inputMinutes
                        to: page.info.maxMinutes
                    }
                    InputField {
                        id: source
                        Layout.fillWidth: true
                        placeholderText: kindFlow.kind === "island" ? "What did you do? (optional)" : "No Hay Tos, Friends… (optional)"
                        maximumLength: page.info.maxText
                        onAccepted: logInputButton.clicked()
                    }
                }
                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: 4
                    spacing: 14

                    PrimaryButton {
                        id: logInputButton
                        text: "Log " + inputMinutes.value + " min"
                        onClicked: {
                            page.learner.logInput(inputMinutes.value, kindFlow.kind, source.text)
                            source.clear()
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: "Logged by mistake? Delete it under Recent activity."
                        color: Theme.muted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Recent activity"
                subtitle: page.info.log.length > 0 ? "Latest entries" : ""
                icon: "calendar"

                RowLayout {
                    Layout.fillWidth: true
                    visible: page.info.log.length === 0
                    spacing: 16

                    Image {
                        source: parent.visible ? "assets/illustrations/empty-immersion.svg" : ""
                        sourceSize: Qt.size(120, 90)
                        Layout.preferredWidth: 120
                        Layout.preferredHeight: 90
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            text: "No immersion sessions logged yet."
                            color: Theme.ink
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: "What you log above appears here, newest first."
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                        }
                    }
                }
                Repeater {
                    model: page.info.log

                    delegate: LogLine {
                        required property var modelData
                        date: modelData.date
                        minutes: modelData.minutes
                        detail: modelData.kind + (modelData.source !== "" ? " · " + modelData.source : "")
                                + (modelData.imported ? " (imported, no XP)" : "")
                        removable: true
                        onRemove: page.learner.removeInput(modelData.index)
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: !page.wide
            Layout.preferredWidth: page.wide ? 400 : -1
            Layout.alignment: Qt.AlignTop
            spacing: 20

            Card {
                Layout.fillWidth: true
                title: "Listening milestones"
                subtitle: page.info.nextHours > 0 ? "Next: " + page.info.nextHours + " h" : "Every milestone passed"
                icon: "chart"

                Text {
                    text: page.info.listeningHours + " h"
                    color: Theme.ink
                    font.pixelSize: 34
                    font.weight: Font.Bold
                }
                Meter {
                    visible: page.info.nextHours > 0
                    value: page.info.levelProgress
                }
                Text {
                    Layout.fillWidth: true
                    text: page.info.nextHours > 0
                          ? "Dreaming Spanish's roadmap measures progress in hours of listening you mostly understand. You are at "
                            + "their level " + page.info.level + "; level " + (page.info.level + 1) + " starts at "
                            + page.info.nextHours + " hours."
                          : "You passed the last Dreaming Spanish milestone, 1,500 hours."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    Layout.fillWidth: true
                    text: "A listening reference, not a CEFR level. The language island and conversations count as "
                          + "Spanish time, not as listening hours."
                    color: Theme.faint
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }

            Card {
                Layout.fillWidth: true
                title: "Series without English"
                icon: "lightbulb"

                Repeater {
                    model: page.info.seriesTips

                    delegate: RowLayout {
                        id: tip
                        required property string modelData
                        Layout.fillWidth: true
                        spacing: 10

                        Rectangle {
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 7
                            implicitWidth: 6
                            implicitHeight: 6
                            radius: 3
                            color: Theme.accent
                        }
                        Text {
                            Layout.fillWidth: true
                            text: tip.modelData
                            color: Theme.ink
                            font.pixelSize: 14
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                    }
                }
            }
        }
    }

    // Series made in Spanish by level: the learner's own level first, the others a tab away.
    property string seriesPicked: ""  // "" follows the learner's level
    readonly property string seriesTab: seriesPicked !== "" ? seriesPicked : (info.seriesLevel ?? "A1")
    readonly property var seriesShown: (info.series ?? []).filter(s => s.level === seriesTab)
    readonly property var levelOrder: ["A1", "A2", "B1"]
    readonly property int seriesColumns: columnWidth >= 900 ? 3 : columnWidth >= 600 ? 2 : 1
    // The card's inner width (its padding is 22 on each side) shared by the columns and their 12 px gaps.
    readonly property real seriesCellWidth: (columnWidth - 44 - 12 * (seriesColumns - 1)) / seriesColumns

    Card {
        id: seriesCard
        Layout.fillWidth: true
        title: "Series in Spanish"
        subtitle: "Your level: " + (page.info.seriesLevel ?? "A1")
        icon: "play"

        Text {
            Layout.fillWidth: true
            text: "Series picked for the level you are at, so you follow most of what you hear. " + (page.info.seriesWhy ?? "")
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: page.levelOrder

                delegate: Chip {
                    required property string modelData
                    text: modelData + (modelData === page.info.seriesLevel ? " · your level" : "")
                    checked: page.seriesTab === modelData
                    onClicked: page.seriesPicked = modelData === page.info.seriesLevel ? "" : modelData
                }
            }
        }
        Text {
            Layout.fillWidth: true
            visible: page.levelOrder.indexOf(page.seriesTab) > page.levelOrder.indexOf(page.info.seriesLevel ?? "A1")
            text: "For later: these will be easier to follow once you reach " + page.seriesTab + "."
            color: Theme.accentStrong
            font.pixelSize: 13
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
        }
        GridLayout {
            Layout.fillWidth: true
            columns: page.seriesColumns
            columnSpacing: 12
            rowSpacing: 12

            Repeater {
                model: page.seriesShown

                delegate: Rectangle {
                    id: seriesTile
                    required property var modelData
                    // A column wide even when a level has a single series.
                    Layout.preferredWidth: page.seriesCellWidth
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignTop
                    implicitHeight: seriesColumn.implicitHeight + 32
                    radius: Theme.smallRadius
                    color: Theme.raised
                    border.color: Theme.edge

                    ColumnLayout {
                        id: seriesColumn
                        anchors { left: parent.left; right: parent.right; top: parent.top; margins: 16 }
                        spacing: 6

                        Text {
                            Layout.fillWidth: true
                            text: seriesTile.modelData.name
                            color: Theme.ink
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                        Text {
                            Layout.fillWidth: true
                            text: seriesTile.modelData.level + " · " + seriesTile.modelData.kind + " · " + seriesTile.modelData.years
                            color: Theme.accentStrong
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.topMargin: 4
                            text: seriesTile.modelData.note
                            color: Theme.muted
                            font.pixelSize: 13
                            lineHeight: 1.15
                            wrapMode: Text.WordWrap
                        }
                        Flow {
                            Layout.fillWidth: true
                            Layout.topMargin: 6
                            spacing: 6

                            Repeater {
                                model: seriesTile.modelData.where

                                delegate: Rectangle {
                                    id: service
                                    required property var modelData
                                    implicitWidth: serviceText.implicitWidth + 16
                                    implicitHeight: 24
                                    radius: 12
                                    color: Theme.panel
                                    border.color: Theme.line

                                    Text {
                                        id: serviceText
                                        anchors.centerIn: parent
                                        text: service.modelData.service
                                              + (service.modelData.detail !== "" ? " · " + service.modelData.detail : "")
                                        color: Theme.ink
                                        font.pixelSize: 12
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        Text {
            Layout.fillWidth: true
            text: (page.info.seriesWhere ?? "") + " All of these are from Spain, with the Castilian accent and vosotros, "
                  + "which the course doesn't teach: good extra listening, not a model to copy."
            color: Theme.faint
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }

    // What was watched on Netflix and the like, from the history file the service lets the learner download.
    FileDialog {
        id: viewingDialog
        title: "Open your viewing history"
        fileMode: FileDialog.OpenFile
        nameFilters: ["Viewing history (*.csv)", "All files (*)"]
        onAccepted: page.openViewing(viewingDialog.selectedFile)
    }

    // readViewing()'s answer while the learner picks shows; null before a file is opened and after importing.
    property var viewing: null
    // The shows ticked as watched in Spanish, by name.
    property var picked: ({})
    property string viewingProfile: ""
    property bool viewingBeforeStart: false
    property string viewingMessage: ""
    property bool viewingFailed: false
    property bool viewingNote: false  // a message that is neither a success nor an error
    // Long histories list their most watched shows first and the rest on request.
    property bool viewingAll: false
    readonly property int viewingShown: 12
    readonly property int pickedCount: Object.keys(picked).length
    // Made in Spanish and not ticked yet, for the one-click button.
    readonly property var spanishUnticked: viewing ? viewing.shows.filter(s => s.spanish && !picked[s.show]).map(s => s.show)
                                                   : []
    // A Netflix history waiting in Downloads ({} when none). On Windows it is looked for when the page opens and each
    // time the app comes back to the front, which is when the learner returns from downloading it. On the Mac the
    // first look makes macOS ask for the Downloads folder, so it waits until the learner presses Look in Downloads.
    readonly property bool mac: Qt.platform.os === "osx"
    property bool watchDownloads: !mac
    property var download: watchDownloads ? learner.downloadedViewing() : ({})
    Connections {
        target: Qt.application
        function onStateChanged() {
            if (Qt.application.state === Qt.ApplicationActive && page.watchDownloads && !page.viewing)
                page.download = page.learner.downloadedViewing()
        }
    }
    function checkDownloads(asked) {
        watchDownloads = true
        download = learner.downloadedViewing()
        if (asked && download.url === undefined) {
            viewingFailed = false
            viewingNote = true
            viewingMessage = "No Netflix history with new viewing in Downloads from the last two weeks. Download it "
                    + "again, or press Choose a file… to pick it yourself."
                    + (mac ? " If macOS said Don't Allow, see the guide below." : "")
        } else if (asked) {
            viewingMessage = ""
        }
    }

    // The step-by-step guide, open on the learner's own system.
    property bool guideOpen: false
    property bool guideMac: mac
    readonly property var windowsGuide: [
        { title: "1. Open Netflix on this computer",
          body: "Use Edge, Chrome or Firefox: the Netflix app, the phone and the TV have no download. Press Open "
                + "Netflix above, sign in if asked, and choose your profile. You land on Viewing activity." },
        { title: "2. Download all",
          body: "Scroll to the very bottom of the list and press Download all. The browser saves "
                + "NetflixViewingHistory.csv in your Downloads folder (Ctrl+J shows it)." },
        { title: "3. Come back to LearnQuick",
          body: "It looks in Downloads by itself and shows Your Netflix history is in Downloads. Press Open it. "
                + "Nothing appears? Press Look in Downloads, or Choose a file…, or drag the file from File Explorer "
                + "onto this card." },
        { title: "4. Tick and import",
          body: "Tick the shows you watched with Spanish audio (Tick the … made in Spanish helps), set the minutes "
                + "per episode and press Import. Next time, the same shows come back ticked and only new episodes "
                + "are added." },
        { title: "With real minutes and every profile",
          body: "Ask Netflix for your data at netflix.com/account/getmyinfo. When the email arrives (it can take up "
                + "to 30 days), download the zip, right-click it in Downloads → Extract All… → Extract. LearnQuick "
                + "finds netflix-report\\CONTENT_INTERACTION\\ViewingActivity.csv in the new folder." },
    ]
    readonly property var macGuide: [
        { title: "1. Open Netflix on this Mac",
          body: "Use Safari, Chrome or Firefox: the Netflix app on iPhone, iPad or the TV has no download. Press Open "
                + "Netflix above, sign in if asked, and choose your profile. You land on Viewing activity." },
        { title: "2. Download all",
          body: "Scroll to the very bottom of the list and press Download all. Safari saves NetflixViewingHistory.csv "
                + "in Downloads (the arrow at the top right of Safari shows it)." },
        { title: "3. Come back and press Look in Downloads",
          body: "The first time, macOS asks whether LearnQuick may access files in your Downloads folder. Press "
                + "Allow: the app only looks for the Netflix file, and reads it on this Mac. Then press Open it." },
        { title: "4. Tick and import",
          body: "Tick the shows you watched with Spanish audio (Tick the … made in Spanish helps), set the minutes "
                + "per episode and press Import. Next time, the same shows come back ticked and only new episodes "
                + "are added." },
        { title: "Pressed Don't Allow?",
          body: "Open System Settings → Privacy & Security → Files & Folders → LearnQuick and turn on Downloads "
                + "Folder. Or skip the permission: press Choose a file…, or drag the file from Finder onto this card." },
        { title: "With real minutes and every profile",
          body: "Ask Netflix for your data at netflix.com/account/getmyinfo. When the email arrives (it can take up "
                + "to 30 days), download the zip. Safari unzips it into a netflix-report folder in Downloads by itself; "
                + "with Chrome or Firefox, double-click the zip in Finder. Then press Look in Downloads." },
    ]

    function openViewing(file) {
        const read = learner.readViewing(file)
        viewingAll = false
        viewingBeforeStart = false
        viewingProfile = ""
        if (read.error !== undefined) {
            viewing = null
            viewingNote = false
            viewingFailed = true
            viewingMessage = read.error
            return
        }
        viewingFailed = false
        viewingNote = false
        viewingMessage = ""
        // Shows imported before stay ticked; nothing else is, because no file says which audio was on.
        const ticked = {}
        for (const show of read.shows) {
            if (show.before)
                ticked[show.show] = true
        }
        picked = ticked
        viewing = read
    }
    function tickSpanish() {
        const next = Object.assign({}, picked)
        for (const show of spanishUnticked)
            next[show] = true
        picked = next
    }
    function togglePick(show) {
        const next = Object.assign({}, picked)
        if (next[show])
            delete next[show]
        else
            next[show] = true
        picked = next
    }
    function importPicked() {
        const done = learner.importViewing(Object.keys(picked), viewingProfile, viewingMinutes.value,
                                           viewingBeforeStart)
        viewing = null
        picked = {}
        if (watchDownloads)
            download = learner.downloadedViewing()
        viewingNote = false
        viewingFailed = done.error !== undefined
        if (viewingFailed)
            viewingMessage = done.error
        else if (done.added === 0)
            viewingMessage = done.already > 0 ? "Nothing new: everything you picked was already in your log."
                                              : "Nothing to add: the shows you picked have no viewing in those days."
        else
            viewingMessage = "Added " + done.added + (done.added === 1 ? " episode" : " episodes") + ", "
                    + (done.minutes >= 120 ? (done.minutes / 60).toFixed(1) + " hours" : done.minutes + " minutes")
                    + " of listening, on the days you watched."
                    + (done.already > 0 ? " " + done.already + " already in your log were left as they were." : "")
    }

    Card {
        id: viewingCard
        Layout.fillWidth: true
        border.color: viewingDrop.containsDrag ? Theme.accent : Theme.line
        border.width: viewingDrop.containsDrag ? 2 : 1
        title: "Import what you watched"
        subtitle: page.viewing ? page.viewing.service + " history" : "Netflix and other services"
        icon: "download"

        // A history dropped anywhere on the card opens like a chosen one.
        DropArea {
            id: viewingDrop
            parent: viewingCard
            anchors.fill: parent
            onEntered: drag => drag.accepted = drag.hasUrls
            onDropped: drop => {
                if (drop.hasUrls) {
                    page.openViewing(drop.urls[0])
                    drop.accept()
                }
            }
        }

        // Before a file is opened: a history waiting in Downloads, or three steps to get one.
        ColumnLayout {
            Layout.fillWidth: true
            visible: !page.viewing
            spacing: 14

            Rectangle {
                Layout.fillWidth: true
                visible: page.download.url !== undefined
                implicitHeight: foundRow.implicitHeight + 28
                radius: Theme.smallRadius
                color: Theme.accentSoft
                border.color: Theme.accentLine

                RowLayout {
                    id: foundRow
                    anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
                    spacing: 14

                    Icon {
                        name: "download"
                        size: 22
                        color: Theme.accent
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Text {
                            Layout.fillWidth: true
                            text: "Your Netflix history is in Downloads"
                            color: Theme.ink
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                        }
                        Text {
                            Layout.fillWidth: true
                            text: (page.download.name ?? "") + ", saved " + (page.download.saved ?? "")
                                  + ". It has viewing you haven't imported yet."
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                    }
                    PrimaryButton {
                        text: "Open it"
                        onClicked: page.openViewing(page.download.url)
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: "Streaming services don't let other apps into your account, and LearnQuick never asks for your "
                      + "password. Three steps instead, about a minute:"
                color: Theme.muted
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            GridLayout {
                Layout.fillWidth: true
                columns: page.wide ? 3 : 1
                columnSpacing: 20
                rowSpacing: 14

                Repeater {
                    model: [
                        { title: "Open your viewing activity", link: true,
                          steps: "Sign in to Netflix if it asks, and pick your profile." },
                        { title: "Press Download all", link: false,
                          steps: "At the very bottom of the list. Your browser saves NetflixViewingHistory.csv "
                                 + "in Downloads." },
                        { title: page.mac ? "Come back and look" : "Come back here", link: false,
                          steps: page.mac ? "Press Look in Downloads below; the first time, macOS asks: press "
                                            + "Allow. The file is read on this Mac and sent nowhere."
                                          : "LearnQuick finds the file in Downloads by itself and offers to open "
                                            + "it. It is read on this computer and sent nowhere." },
                    ]

                    delegate: RowLayout {
                        id: howTo
                        required property var modelData
                        required property int index
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        Layout.alignment: Qt.AlignTop
                        spacing: 12

                        Rectangle {
                            Layout.alignment: Qt.AlignTop
                            implicitWidth: 28
                            implicitHeight: 28
                            radius: 14
                            color: Theme.accentSoft

                            Text {
                                anchors.centerIn: parent
                                text: howTo.index + 1
                                color: Theme.accentStrong
                                font.pixelSize: 14
                                font.weight: Font.Bold
                            }
                        }
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Text {
                                Layout.fillWidth: true
                                text: howTo.modelData.title
                                color: Theme.ink
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                Layout.fillWidth: true
                                text: howTo.modelData.steps
                                color: Theme.muted
                                font.pixelSize: 13
                                wrapMode: Text.WordWrap
                            }
                            LinkButton {
                                Layout.topMargin: 4
                                visible: howTo.modelData.link
                                text: "Open Netflix"
                                url: "https://www.netflix.com/viewingactivity"
                            }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                PrimaryButton {
                    secondary: true
                    text: "Look in Downloads"
                    onClicked: page.checkDownloads(true)
                }
                PrimaryButton {
                    secondary: true
                    text: "Choose a file…"
                    onClicked: viewingDialog.open()
                }
                Text {
                    Layout.fillWidth: true
                    text: "or drop it on this card."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }

            // The whole way, for Windows and for the Mac: the downloads, the Mac's permission and what to do when
            // something isn't where it should be.
            AbstractButton {
                id: guideToggle
                implicitHeight: 32
                focusPolicy: Qt.StrongFocus
                Accessible.name: guideText.text
                onClicked: page.guideOpen = !page.guideOpen
                background: null
                contentItem: RowLayout {
                    spacing: 6

                    Icon {
                        name: page.guideOpen ? "chevron-left" : "chevron-right"
                        rotation: page.guideOpen ? -90 : 0
                        size: 16
                        color: Theme.accentStrong
                    }
                    Text {
                        id: guideText
                        text: page.guideOpen ? "Hide the step-by-step guide" : "Step-by-step guide for Windows and Mac"
                        color: Theme.accentStrong
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        font.underline: guideToggle.hovered || guideToggle.visualFocus
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                visible: page.guideOpen
                implicitHeight: guideColumn.implicitHeight + 36
                radius: Theme.smallRadius
                color: Theme.raised
                border.color: Theme.edge

                ColumnLayout {
                    id: guideColumn
                    anchors { left: parent.left; right: parent.right; top: parent.top; margins: 18 }
                    spacing: 14

                    Flow {
                        Layout.fillWidth: true
                        spacing: 8

                        Chip {
                            text: "Windows"
                            checked: !page.guideMac
                            onClicked: page.guideMac = false
                        }
                        Chip {
                            text: "Mac"
                            checked: page.guideMac
                            onClicked: page.guideMac = true
                        }
                    }
                    Repeater {
                        model: page.guideMac ? page.macGuide : page.windowsGuide

                        delegate: ColumnLayout {
                            id: guideStep
                            required property var modelData
                            Layout.fillWidth: true
                            spacing: 3

                            Text {
                                Layout.fillWidth: true
                                text: guideStep.modelData.title
                                color: Theme.ink
                                font.pixelSize: 14
                                font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap
                            }
                            Text {
                                Layout.fillWidth: true
                                text: guideStep.modelData.body
                                color: Theme.muted
                                font.pixelSize: 13
                                lineHeight: 1.15
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: "HBO Max, Prime Video and Disney+ have no history download: a spreadsheet saved as CSV with Title "
                      + "and Date columns, and Minutes if you know them, opens here too."
                color: Theme.faint
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
        }

        // Once a file is open: its shows, to tick the ones watched in Spanish.
        ColumnLayout {
            Layout.fillWidth: true
            visible: page.viewing !== null
            spacing: 12

            Text {
                Layout.fillWidth: true
                text: page.viewing
                      ? page.viewing.rows + " views from " + page.viewing.first + " to " + page.viewing.last
                        + (page.viewing.skipped > 0 ? " (" + page.viewing.skipped + " trailers, previews and unreadable "
                                                      + "rows left out)" : "") + "."
                      : ""
                color: Theme.ink
                font.pixelSize: 14
                wrapMode: Text.WordWrap
            }
            Flow {
                Layout.fillWidth: true
                visible: page.viewing !== null && page.viewing.profiles.length > 1
                spacing: 8

                Chip {
                    text: "Every profile"
                    checked: page.viewingProfile === ""
                    onClicked: page.viewingProfile = ""
                }
                Repeater {
                    model: page.viewing ? page.viewing.profiles : []

                    delegate: Chip {
                        required property string modelData
                        text: modelData
                        checked: page.viewingProfile === modelData
                        onClicked: page.viewingProfile = modelData
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: "Tick the shows you watched with Spanish audio. Shows made in Spanish are marked, but Netflix "
                      + "plays your profile's language by default, so only you know which audio was on."
                color: Theme.muted
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
            PrimaryButton {
                visible: page.spanishUnticked.length > 0
                secondary: true
                text: "Tick the " + page.spanishUnticked.length + " made in Spanish"
                onClicked: page.tickSpanish()
            }
            GridLayout {
                Layout.fillWidth: true
                columns: page.wide ? 2 : 1
                columnSpacing: 10
                rowSpacing: 8

                Repeater {
                    model: page.viewing
                           ? (page.viewingAll ? page.viewing.shows : page.viewing.shows.slice(0, page.viewingShown))
                           : []

                    delegate: AbstractButton {
                        id: showButton
                        required property var modelData
                        readonly property bool ticked: page.picked[modelData.show] === true
                        Layout.fillWidth: true
                        Layout.preferredWidth: 1
                        implicitHeight: showRow.implicitHeight
                        padding: 0
                        focusPolicy: Qt.StrongFocus
                        Accessible.role: Accessible.CheckBox
                        Accessible.checked: ticked
                        Accessible.name: modelData.show
                        onClicked: page.togglePick(modelData.show)

                        background: Rectangle {
                            radius: Theme.smallRadius
                            color: showButton.ticked ? Theme.accentSoft : showButton.hovered ? Theme.hover : Theme.raised
                            border.color: showButton.visualFocus || showButton.ticked ? Theme.accent : Theme.edge
                            border.width: showButton.visualFocus ? 2 : 1
                        }
                        contentItem: RowLayout {
                            id: showRow
                            spacing: 12

                            Rectangle {
                                Layout.leftMargin: 12
                                implicitWidth: 20
                                implicitHeight: 20
                                radius: 5
                                color: showButton.ticked ? Theme.accent : "transparent"
                                border.color: showButton.ticked ? Theme.accent : Theme.faint
                                border.width: 2

                                Icon {
                                    anchors.centerIn: parent
                                    visible: showButton.ticked
                                    name: "check"
                                    size: 13
                                    color: Theme.inkOnAccent
                                }
                            }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: 10
                                Layout.bottomMargin: 10
                                spacing: 2

                                Text {
                                    Layout.fillWidth: true
                                    text: showButton.modelData.show
                                    color: Theme.ink
                                    font.pixelSize: 14
                                    font.weight: Font.DemiBold
                                    elide: Text.ElideRight
                                    textFormat: Text.PlainText
                                }
                                Text {
                                    Layout.fillWidth: true
                                    text: {
                                        const s = showButton.modelData
                                        const titles = s.titles === 1 ? "1 title" : s.titles + " titles"
                                        const when = s.first === s.last ? s.last : s.first + " – " + s.last
                                        return titles + " · " + when + (s.before ? " · imported before" : "")
                                    }
                                    color: Theme.muted
                                    font.pixelSize: 12
                                    elide: Text.ElideRight
                                }
                            }
                            Rectangle {
                                Layout.rightMargin: 12
                                visible: showButton.modelData.spanish
                                implicitWidth: spanishLabel.implicitWidth + 16
                                implicitHeight: 22
                                radius: 11
                                color: Theme.successSoft
                                border.color: Theme.goodLine

                                Text {
                                    id: spanishLabel
                                    anchors.centerIn: parent
                                    text: "Made in Spanish"
                                    color: Theme.goodInk
                                    font.pixelSize: 11
                                    font.weight: Font.DemiBold
                                }
                            }
                            Item {
                                visible: !showButton.modelData.spanish
                                implicitWidth: 0
                                Layout.rightMargin: 12
                            }
                        }
                    }
                }
            }
            AbstractButton {
                id: moreShows
                visible: page.viewing !== null && !page.viewingAll
                         && page.viewing.shows.length > page.viewingShown
                implicitHeight: 32
                focusPolicy: Qt.StrongFocus
                onClicked: page.viewingAll = true
                contentItem: Text {
                    text: page.viewing ? "Show all " + page.viewing.shows.length + " shows and films" : ""
                    color: Theme.accentStrong
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    font.underline: moreShows.hovered || moreShows.visualFocus
                }
                background: null
            }

            // Without durations in the file, each episode counts the minutes given here.
            RowLayout {
                Layout.fillWidth: true
                visible: page.viewing !== null && !page.viewing.hasMinutes
                spacing: 14

                Text {
                    text: "Minutes per episode"
                    color: Theme.ink
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                }
                MinutesField {
                    id: viewingMinutes
                    value: 40
                    from: 5
                    to: 180
                }
                Text {
                    Layout.fillWidth: true
                    text: "This file has no durations."
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
            }
            AbstractButton {
                id: beforeStart
                Layout.fillWidth: true
                implicitHeight: beforeRow.implicitHeight + 8
                focusPolicy: Qt.StrongFocus
                Accessible.role: Accessible.CheckBox
                Accessible.checked: page.viewingBeforeStart
                Accessible.name: beforeText.text
                onClicked: page.viewingBeforeStart = !page.viewingBeforeStart
                background: null
                contentItem: RowLayout {
                    id: beforeRow
                    spacing: 10

                    Rectangle {
                        implicitWidth: 20
                        implicitHeight: 20
                        radius: 5
                        color: page.viewingBeforeStart ? Theme.accent : "transparent"
                        border.color: beforeStart.visualFocus ? Theme.accentStrong
                                                              : page.viewingBeforeStart ? Theme.accent : Theme.faint
                        border.width: 2

                        Icon {
                            anchors.centerIn: parent
                            visible: page.viewingBeforeStart
                            name: "check"
                            size: 13
                            color: Theme.inkOnAccent
                        }
                    }
                    Text {
                        id: beforeText
                        Layout.fillWidth: true
                        text: page.viewing ? "Also count what you watched before your plan began on "
                                             + page.viewing.planStart : ""
                        color: Theme.ink
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                    }
                }
            }
            Text {
                Layout.fillWidth: true
                text: "Imported viewing counts as listening hours and as Spanish time on the day you watched. It "
                      + "earns no XP, so it never changes your streak after the fact."
                color: Theme.faint
                font.pixelSize: 12
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                PrimaryButton {
                    text: page.pickedCount === 0 ? "Tick a show to import"
                                                 : "Import " + page.pickedCount
                                                   + (page.pickedCount === 1 ? " show" : " shows")
                    enabled: page.pickedCount > 0
                    onClicked: page.importPicked()
                }
                PrimaryButton {
                    secondary: true
                    text: "Cancel"
                    onClicked: {
                        page.viewing = null
                        page.picked = {}
                    }
                }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: page.viewingMessage !== ""
            text: page.viewingMessage
            color: page.viewingFailed ? Theme.danger : page.viewingNote ? Theme.muted : Theme.goodInk
            font.pixelSize: 14
            font.weight: Font.DemiBold
            wrapMode: Text.WordWrap
            textFormat: Text.PlainText
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Recommended listening, A2 → B1"
        subtitle: "Opens in your browser"
        icon: "play"

        Text {
            Layout.fillWidth: true
            text: "Choose something you understand at least 70–80 % of. Slow it to 0.9× if it feels too fast."
            color: Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
        GridLayout {
            Layout.fillWidth: true
            columns: page.columnWidth >= 1100 ? 3 : page.columnWidth >= 640 ? 2 : 1
            rowSpacing: 10
            columnSpacing: 10

            Repeater {
                model: page.info.resources

                delegate: AbstractButton {
                    id: resource
                    required property var modelData
                    Layout.fillWidth: true
                    Layout.preferredWidth: 1
                    Layout.fillHeight: true
                    implicitHeight: resourceColumn.implicitHeight + 28
                    focusPolicy: Qt.StrongFocus
                    onClicked: Qt.openUrlExternally(modelData.url)

                    background: Rectangle {
                        radius: Theme.smallRadius
                        color: resource.hovered ? Theme.hover : Theme.raised
                        border.color: resource.visualFocus ? Theme.accent : resource.hovered ? Theme.hoverLine : Theme.edge
                        border.width: resource.visualFocus ? 2 : 1
                    }
                    contentItem: ColumnLayout {
                        id: resourceColumn
                        spacing: 6

                        RowLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 14
                            Layout.rightMargin: 14
                            Layout.topMargin: 14

                            Text {
                                Layout.fillWidth: true
                                text: resource.modelData.name
                                color: Theme.ink
                                font.pixelSize: 15
                                font.weight: Font.DemiBold
                                wrapMode: Text.WordWrap
                            }
                            Icon {
                                Layout.alignment: Qt.AlignTop
                                name: "external"
                                size: 16
                                color: resource.hovered ? Theme.accentStrong : Theme.faint
                            }
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.leftMargin: 14
                            Layout.rightMargin: 14
                            text: resource.modelData.level + " · " + resource.modelData.kind + " · " + resource.modelData.accent
                            color: Theme.accentStrong
                            font.pixelSize: 12
                            font.weight: Font.DemiBold
                            wrapMode: Text.WordWrap
                        }
                        Text {
                            Layout.fillWidth: true
                            Layout.leftMargin: 14
                            Layout.rightMargin: 14
                            Layout.bottomMargin: 14
                            text: resource.modelData.note
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Goodbye English: your devices"
        icon: "monitor"
        subtitle: page.info.missionsDone + " / " + page.info.missions.length

        Text {
            Layout.fillWidth: true
            text: "Every notification and menu becomes a tiny Spanish lesson. Tick each one when it is done."
            color: Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
        Repeater {
            model: page.info.missions

            delegate: AbstractButton {
                id: mission
                required property var modelData
                Layout.fillWidth: true
                implicitHeight: missionRow.implicitHeight + 24
                focusPolicy: Qt.StrongFocus
                Accessible.role: Accessible.CheckBox
                Accessible.checked: modelData.done
                onClicked: page.learner.setMission(modelData.id, !modelData.done)

                background: Rectangle {
                    radius: Theme.smallRadius
                    color: mission.modelData.done ? Theme.successSoft : mission.hovered ? Theme.hover : Theme.raised
                    border.color: mission.visualFocus ? Theme.accent : mission.modelData.done ? Theme.goodLine : Theme.edge
                    border.width: mission.visualFocus ? 2 : 1
                }
                contentItem: RowLayout {
                    id: missionRow
                    spacing: 12

                    Rectangle {
                        Layout.leftMargin: 14
                        Layout.alignment: Qt.AlignTop
                        Layout.topMargin: 12
                        implicitWidth: 22
                        implicitHeight: 22
                        radius: 6
                        color: mission.modelData.done ? Theme.success : "transparent"
                        border.color: mission.modelData.done ? Theme.success : Theme.faint
                        border.width: 2

                        Icon {
                            anchors.centerIn: parent
                            visible: mission.modelData.done
                            name: "check"
                            size: 14
                            color: Theme.inkOnAccent
                        }
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.rightMargin: 14
                        Layout.topMargin: 12
                        Layout.bottomMargin: 12
                        spacing: 3

                        Text {
                            text: mission.modelData.title
                            color: Theme.ink
                            font.pixelSize: 15
                            font.weight: Font.DemiBold
                        }
                        Text {
                            Layout.fillWidth: true
                            text: mission.modelData.how
                            color: Theme.muted
                            font.pixelSize: 13
                            wrapMode: Text.WordWrap
                            textFormat: Text.PlainText
                        }
                    }
                }
            }
        }
    }

    Card {
        Layout.fillWidth: true
        title: "Language exchanges"
        icon: "users"
        subtitle: page.info.exchangeCount === 1 ? "1 conversation" : page.info.exchangeCount + " conversations"

        Text {
            Layout.fillWidth: true
            text: "On Tandem or HelloTalk, Spanish speakers practise with you in exchange for English. Aim for one or "
                  + "two a week from phase 2."
            color: Theme.muted
            font.pixelSize: 13
            wrapMode: Text.WordWrap
        }
        RowLayout {
            spacing: 10

            LinkButton {
                text: "Tandem"
                url: "https://www.tandem.net"
            }
            LinkButton {
                text: "HelloTalk"
                url: "https://www.hellotalk.com"
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            MinutesField {
                id: talkMinutes
                value: 30
                from: 5
                to: page.info.maxExchangeMinutes
            }
            InputField {
                id: partner
                Layout.fillWidth: true
                placeholderText: "Who with? (optional)"
                maximumLength: page.info.maxText
            }
        }
        InputField {
            id: note
            Layout.fillWidth: true
            placeholderText: "What did you talk about? (optional)"
            maximumLength: page.info.maxText
            onAccepted: logTalkButton.clicked()
        }
        PrimaryButton {
            id: logTalkButton
            text: "Log " + talkMinutes.value + " min of conversation"
            onClicked: {
                page.learner.logExchange(talkMinutes.value, partner.text, note.text)
                partner.clear()
                note.clear()
            }
        }
        Repeater {
            model: page.info.exchanges

            delegate: LogLine {
                required property var modelData
                date: modelData.date
                minutes: modelData.minutes
                detail: [modelData.partner, modelData.note].filter(s => s !== "").join(" · ")
            }
        }

        Text {
            Layout.topMargin: 6
            text: "Phrases for your first exchange"
            color: Theme.ink
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }
        Repeater {
            model: page.info.phrases

            delegate: RowLayout {
                id: phrase
                required property var modelData
                Layout.fillWidth: true
                spacing: 12

                SpeakButton {
                    spanish: phrase.modelData.es
                    diameter: 32
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 1

                    Text {
                        Layout.fillWidth: true
                        text: phrase.modelData.es
                        color: Theme.ink
                        font.pixelSize: 15
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                    Text {
                        Layout.fillWidth: true
                        text: phrase.modelData.en
                        color: Theme.muted
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                }
            }
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.topMargin: 4
            implicitHeight: safety.implicitHeight + 24
            radius: Theme.smallRadius
            color: Theme.blueSoft

            Text {
                id: safety
                anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 12 }
                text: "<b>Stay safe:</b> keep chats inside the app at first, don't share your address, workplace or "
                      + "financial details, and leave any conversation that makes you uncomfortable."
                textFormat: Text.StyledText
                color: Theme.ink
                font.pixelSize: 13
                wrapMode: Text.WordWrap
            }
        }
    }
}
