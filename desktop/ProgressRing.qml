import QtQuick
import QtQuick.Shapes

// A ring that fills clockwise from the top, with the value in words in the middle. `value` runs from 0 to 1.
Item {
    id: ring

    property real value: 0
    property string label
    property string caption
    property real thickness: 9
    property color tint: Theme.accent

    readonly property real clamped: Math.max(0, Math.min(1, value))

    implicitWidth: 104
    implicitHeight: 104

    Accessible.role: Accessible.ProgressBar
    Accessible.name: ring.label + " " + ring.caption

    Shape {
        anchors.fill: parent
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: Theme.line
            strokeWidth: ring.thickness
            fillColor: "transparent"
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: (Math.min(ring.width, ring.height) - ring.thickness) / 2
                radiusY: radiusX
                startAngle: 0
                sweepAngle: 360
            }
        }
        ShapePath {
            strokeColor: ring.clamped > 0 ? ring.tint : "transparent"
            strokeWidth: ring.thickness
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            PathAngleArc {
                centerX: ring.width / 2
                centerY: ring.height / 2
                radiusX: (Math.min(ring.width, ring.height) - ring.thickness) / 2
                radiusY: radiusX
                startAngle: -90
                // A sliver still shows for the first day, so the ring never looks broken.
                sweepAngle: ring.clamped > 0 ? Math.max(4, 360 * ring.clamped) : 0
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 0

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: ring.label
            color: Theme.ink
            font.pixelSize: 22
            font.weight: Font.Bold
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            visible: ring.caption !== ""
            text: ring.caption
            color: Theme.muted
            font.pixelSize: 12
        }
    }
}
