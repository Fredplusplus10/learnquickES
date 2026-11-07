import QtQuick
import QtQuick.Layouts

// A page's opening block: a small line above, the title, an optional highlighted line and a subtitle, with the
// Spanish-town illustration on the right. Child items (such as the streak) go in a row under the text, so they
// never cover the illustration.
Item {
    id: hero

    property string overline
    property string title
    property string highlight  // shown in the accent colour, before `detail` on the same line
    property string detail
    property string subtitle
    property bool illustration: true
    default property alias footer: footerRow.data

    // The highlighted line is styled text: course titles such as "Plans, Opinions & Politeness" are escaped.
    function escaped(text) {
        return text.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
    }

    // The illustration needs room beside the text; on a narrow window the text keeps the whole width.
    readonly property bool showArt: illustration && width >= 700
    readonly property real artWidth: showArt ? Math.min(480, width * 0.46) : 0

    Layout.fillWidth: true
    implicitHeight: Math.max(textColumn.implicitHeight, showArt ? artWidth * 178 / 480 : 0)

    Image {
        id: art
        visible: hero.showArt
        anchors { right: parent.right; bottom: parent.bottom }
        width: hero.artWidth
        height: width * 178 / 480
        source: "assets/illustrations/spanish-town-banner.svg"
        sourceSize.width: width * 2
        sourceSize.height: height * 2
        fillMode: Image.PreserveAspectFit
        smooth: true
        // Its colours are a sunny day's; at night they step back so the text stays first.
        opacity: Theme.dark ? 0.55 : 1
        Accessible.ignored: true
    }

    ColumnLayout {
        id: textColumn
        anchors { left: parent.left; top: parent.top }
        width: parent.width - (hero.showArt ? hero.artWidth + 16 : 0)
        spacing: 2

        Text {
            visible: hero.overline !== ""
            text: hero.overline
            color: Theme.muted
            font.pixelSize: 14
        }
        Text {
            Layout.fillWidth: true
            text: hero.title
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            maximumLineCount: 2
            elide: Text.ElideRight
            color: Theme.ink
            font.pixelSize: 34
            font.weight: Font.Bold
        }
        Text {
            Layout.fillWidth: true
            Layout.topMargin: 2
            visible: hero.highlight !== "" || hero.detail !== ""
            textFormat: Text.StyledText
            text: "<font color='" + Theme.accentStrong + "'><b>" + hero.escaped(hero.highlight) + "</b></font>"
                  + (hero.detail !== "" ? " · " + hero.escaped(hero.detail) : "")
            color: Theme.ink
            font.pixelSize: 20
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            Layout.topMargin: 4
            visible: hero.subtitle !== ""
            text: hero.subtitle
            textFormat: Text.PlainText
            color: Theme.muted
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }
        RowLayout {
            id: footerRow
            Layout.topMargin: 12
            visible: children.length > 0
            spacing: 10
        }
    }
}
