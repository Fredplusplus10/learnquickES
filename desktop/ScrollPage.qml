import QtQuick
import QtQuick.Controls.Basic
import QtQuick.Layouts

// A screen with a centred, scrollable column. Children go into the column.
FocusScope {
    id: page

    default property alias content: column.data
    property int maxWidth: 860
    signal openRoute(string route)

    function scrollToTop() {
        scroller.contentY = 0
    }
    // Brings an item of the column to the top of the view, or as near as the page's end allows.
    function scrollTo(item) {
        const y = item.mapToItem(column, 0, 0).y + column.y - 16
        scroller.contentY = Math.max(0, Math.min(y, scroller.contentHeight - scroller.height))
    }

    Flickable {
        id: scroller
        anchors.fill: parent
        contentWidth: width
        contentHeight: column.implicitHeight + 64
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        ScrollBar.vertical: ScrollBar {}

        ColumnLayout {
            id: column
            width: Math.min(scroller.width - 64, page.maxWidth)
            x: (scroller.width - width) / 2
            y: 28
            spacing: 20
        }
    }
}
