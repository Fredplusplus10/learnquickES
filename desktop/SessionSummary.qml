import QtQuick
import QtQuick.Layouts

// The end of a session: score, XP, the reflection when it is due, what to look at again and the coach's next
// step. Buttons go in as children; the reflection goes in `reflection`.
ColumnLayout {
    id: summary

    property var result: ({})
    property string title: (result.percent ?? 0) >= 90 ? "¡Excelente!" : (result.percent ?? 0) >= 70 ? "¡Muy bien!" : "¡Sigue así!"
    property string message
    property var nextStep: ({})  // {title, detail, route} from the coach
    default property alias actions: buttons.data
    property alias reflection: reflectionSlot.data
    property bool showReflection: false
    // An empty-state illustration (assets/illustrations/empty-*.svg), shown when there was nothing to do.
    property string illustration
    signal follow(string route)

    readonly property var missed: result.missed ?? []
    readonly property bool scored: (result.answered ?? 0) > 0

    spacing: 18

    RowLayout {
        Layout.fillWidth: true
        spacing: 22

        Image {
            visible: summary.illustration !== ""
            source: visible ? "assets/illustrations/" + summary.illustration + ".svg" : ""
            sourceSize: Qt.size(160, 120)
            Layout.preferredWidth: 160
            Layout.preferredHeight: 120
        }

        // Score ring.
        Item {
            visible: summary.scored
            implicitWidth: 104
            implicitHeight: 104

            Canvas {
                id: ring
                anchors.fill: parent
                readonly property real ratio: (summary.result.percent ?? 0) / 100
                readonly property bool dark: Theme.dark
                onRatioChanged: requestPaint()
                onDarkChanged: requestPaint()
                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.lineWidth = 9
                    ctx.lineCap = "round"
                    ctx.strokeStyle = Theme.line.toString()
                    ctx.beginPath()
                    ctx.arc(width / 2, height / 2, width / 2 - 6, 0, 2 * Math.PI)
                    ctx.stroke()
                    if (ratio > 0) {
                        ctx.strokeStyle = (ratio >= 0.7 ? Theme.success : Theme.accent).toString()
                        ctx.beginPath()
                        ctx.arc(width / 2, height / 2, width / 2 - 6, -Math.PI / 2, -Math.PI / 2 + 2 * Math.PI * ratio)
                        ctx.stroke()
                    }
                }
            }
            Text {
                anchors.centerIn: parent
                text: (summary.result.percent ?? 0) + "%"
                color: Theme.ink
                font.pixelSize: 26
                font.weight: Font.Bold
            }
        }
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Text {
                text: summary.title
                color: Theme.ink
                font.pixelSize: 28
                font.weight: Font.Bold
            }
            Text {
                Layout.fillWidth: true
                visible: text !== ""
                text: summary.message !== "" ? summary.message
                      : summary.scored ? (summary.result.firstTry ?? 0) + " of " + summary.result.answered + " right on the first try"
                      : ""
                color: Theme.muted
                font.pixelSize: 15
                wrapMode: Text.WordWrap
            }
            Text {
                Layout.fillWidth: true
                visible: (summary.result.secondTries ?? 0) > 0
                text: "Second try: " + (summary.result.fixed ?? 0) + " of " + (summary.result.secondTries ?? 0) + " fixed"
                color: Theme.blue
                font.pixelSize: 15
                wrapMode: Text.WordWrap
            }
            RowLayout {
                spacing: 8
                visible: (summary.result.xp ?? 0) > 0

                Icon {
                    name: "bolt"
                    color: Theme.accent
                    size: 18
                }
                Text {
                    text: "+" + (summary.result.xp ?? 0) + " XP"
                    color: Theme.accentStrong
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }
            }
        }
    }

    ColumnLayout {
        id: reflectionSlot
        Layout.fillWidth: true
        visible: summary.showReflection
    }

    // What was missed, with the right answer: the real review material.
    Card {
        visible: summary.missed.length > 0
        Layout.fillWidth: true
        title: "Look at these again"

        Repeater {
            model: summary.missed

            delegate: ColumnLayout {
                id: miss
                required property var modelData
                Layout.fillWidth: true
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    text: miss.modelData.question
                    color: Theme.muted
                    font.pixelSize: 14
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        text: "→ " + miss.modelData.answer
                        color: Theme.ink
                        font.pixelSize: 16
                        font.weight: Font.DemiBold
                        wrapMode: Text.WordWrap
                        textFormat: Text.PlainText
                    }
                    SpeakButton {
                        spanish: miss.modelData.answer
                        diameter: 30
                    }
                }
                Text {
                    Layout.fillWidth: true
                    visible: text !== ""
                    text: miss.modelData.why
                    color: Theme.muted
                    font.pixelSize: 13
                    font.italic: true
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
            }
        }
    }

    // The coach's next step: today's next task, or its first suggestion.
    Rectangle {
        visible: (summary.nextStep.title ?? "") !== ""
        Layout.fillWidth: true
        implicitHeight: nextRow.implicitHeight + 26
        radius: Theme.smallRadius
        color: Theme.panel
        border.color: Theme.line

        RowLayout {
            id: nextRow
            anchors { left: parent.left; right: parent.right; verticalCenter: parent.verticalCenter; margins: 16 }
            spacing: 12

            Icon {
                Layout.alignment: Qt.AlignTop
                name: "route"
                color: Theme.accentStrong
            }
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: "COACH · NEXT STEP"
                    color: Theme.accentStrong
                    font.pixelSize: 11
                    font.weight: Font.Bold
                    font.letterSpacing: 0.8
                }
                Text {
                    Layout.fillWidth: true
                    text: summary.nextStep.title ?? ""
                    color: Theme.ink
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
                Text {
                    Layout.fillWidth: true
                    text: summary.nextStep.detail ?? ""
                    color: Theme.muted
                    font.pixelSize: 13
                    wrapMode: Text.WordWrap
                    textFormat: Text.PlainText
                }
            }
            PrimaryButton {
                visible: (summary.nextStep.route ?? "") !== ""
                secondary: true
                text: "Go"
                onClicked: summary.follow(summary.nextStep.route)
            }
        }
    }

    RowLayout {
        id: buttons
        Layout.fillWidth: true
        spacing: 12
    }
}
