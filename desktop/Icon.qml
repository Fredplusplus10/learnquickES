import QtQuick
import QtQuick.Shapes

// A line icon from the web app's set (app/assets/icons), drawn as a path so it takes any colour.
// Lines, circles and rounded rectangles of the SVG sources are written as path commands.
Item {
    id: icon

    property string name
    property color color: Theme.ink
    property real size: 20

    implicitWidth: size
    implicitHeight: size

    readonly property var paths: ({
        "award": "M6 9a6 6 0 1 0 12 0a6 6 0 1 0 -12 0ZM8 14l-2 8 6-3 6 3-2-8",
        "backspace": "M10 6H19A2 2 0 0 1 21 8V16A2 2 0 0 1 19 18H10L3 12ZM12.5 9.5L17.5 14.5M17.5 9.5L12.5 14.5",
        "bolt": "m14 2-9 12h7l-2 8 9-12h-7l2-8Z",
        "book": "M12 6c-3-2.7-6-3-10-2v15c4-1 7-.5 10 2 3-2.5 6-3 10-2V4c-4-1-7-.7-10 2ZM12 6v15",
        "calendar": "M5 5h14a2 2 0 0 1 2 2v12a2 2 0 0 1 -2 2h-14a2 2 0 0 1 -2 -2v-12a2 2 0 0 1 2 -2ZM7 3v4M17 3v4M3 10h18",
        "cards": "M6 5h12a2 2 0 0 1 2 2v11a2 2 0 0 1 -2 2h-12a2 2 0 0 1 -2 -2v-11a2 2 0 0 1 2 -2ZM8 9h8M8 13h8M8 17h5M7 2h13a2 2 0 0 1 2 2v12",
        "chart": "M3 21h18M5 18v-6h3v6M11 18V5h3v13M17 18V9h3v9",
        "check": "m4 12 5 5L20 6",
        "chevron-left": "m15 5-7 7 7 7",
        "chevron-right": "m9 5 7 7-7 7",
        "clock": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0 -18 0ZM12 7v5l4 2",
        "download": "M12 4v11M7.5 10.5L12 15l4.5-4.5M5 19.5h14",
        "enter": "M18 6V14H7M10 11L7 14M7 14L10 17",
        "external": "M14 4h6v6M20 4L11 13M18 14v4.5a1.5 1.5 0 0 1 -1.5 1.5h-11a1.5 1.5 0 0 1 -1.5 -1.5v-11a1.5 1.5 0 0 1 1.5 -1.5H10",
        "eye": "M2.5 12S6 5.5 12 5.5 21.5 12 21.5 12 18 18.5 12 18.5 2.5 12 2.5 12ZM9 12a3 3 0 1 0 6 0a3 3 0 1 0 -6 0Z",
        "flame": "M12 22c-5.5 0-8-3.6-8-8 0-3.5 2.8-6.5 5-8 .5 2 1.3 3 2.4 3.8C12 7 13.5 5 14 2c3.9 3.2 6 7.5 6 12 0 4.4-2.5 8-8 8Z",
        "globe": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0 -18 0ZM3 12h18M12 3c5 5 5 13 0 18M12 3c-5 5-5 13 0 18",
        "headphones": "M4 14v-3a8 8 0 0 1 16 0v3M5 14h3v7H5a2 2 0 0 1-2-2v-3a2 2 0 0 1 2-2ZM16 14h3a2 2 0 0 1 2 2v3a2 2 0 0 1-2 2h-3v-7Z",
        "home": "m3 10 9-7 9 7v10H3V10ZM9 20v-7h6v7",
        "lightbulb": "M9 18h6M9 21h6M9 15a7 7 0 1 1 6 0l-1 3h-4l-1-3Z",
        "lock": "M7.5 10.5h9a2.5 2.5 0 0 1 2.5 2.5v5a2.5 2.5 0 0 1 -2.5 2.5h-9a2.5 2.5 0 0 1 -2.5 -2.5v-5a2.5 2.5 0 0 1 2.5 -2.5ZM8.5 10.5V7.8a3.5 3.5 0 0 1 7 0v2.7",
        "mic": "M12 2h0a3 3 0 0 1 3 3v6a3 3 0 0 1 -3 3h0a3 3 0 0 1 -3 -3v-6a3 3 0 0 1 3 -3ZM5 11a7 7 0 0 0 14 0M12 18v4M8 22h8",
        "monitor": "M5.6 5h12.8a1.6 1.6 0 0 1 1.6 1.6v7.8a1.6 1.6 0 0 1 -1.6 1.6h-12.8a1.6 1.6 0 0 1 -1.6 -1.6v-7.8a1.6 1.6 0 0 1 1.6 -1.6ZM12 16L12 19M9 19L15 19M8 21L16 21",
        "moon": "M20 14.5A8 8 0 0 1 9.5 4a8 8 0 1 0 10.5 10.5Z",
        "palm": "M12 21c0-5 .5-8.5 2-11.5M14 9.5c-1.8-2.6-4.8-3.4-8-2.3 2.5.4 4.4 1.4 5.6 3M14 9.5c1.1-2.9 3.8-4.4 7-4-2.4.9-4 2.2-4.8 4M14 9.5c2.7.1 4.8 1.6 5.6 4.4-1.9-1.4-3.7-2-5.6-2M8 21h9",
        "pen": "M4 20l1-4.5L15.5 5a2.1 2.1 0 0 1 3 3L8 18.5 4 20ZM13.5 7l3 3",
        "play": "M8 5.5v13l10.5-6.5L8 5.5Z",
        "refresh": "M20 7a9 9 0 0 0-15.4-1.8L3 8M3 3v5h5M4 17a9 9 0 0 0 15.4 1.8L21 16M16 16h5v5",
        "route": "M3 6a2 2 0 1 0 4 0a2 2 0 1 0 -4 0ZM17 18a2 2 0 1 0 4 0a2 2 0 1 0 -4 0ZM7 6h9c5 0 5 7 0 7H9c-5 0-5 5 0 5h8",
        "search": "M4 10.7a6.7 6.7 0 1 0 13.4 0a6.7 6.7 0 1 0 -13.4 0ZM16 16l5 5",
        "settings": "M12 3.5l1.8 2.2 2.7-.3.7 2.8 2.5 1.3-1 2.7 1 2.7-2.5 1.3-.7 2.8-2.7-.3L12 21l-1.8-2.3-2.7.3-.7-2.8-2.5-1.3 1-2.7-1-2.7 2.5-1.3.7-2.8 2.7.3L12 3.5ZM9 12.2a3 3 0 1 0 6 0a3 3 0 1 0 -6 0Z",
        "shield": "M12 3 5 6v5.5c0 4.4 3 7.9 7 9.5 4-1.6 7-5.1 7-9.5V6l-7-3ZM12 8.5l1.1 2.2 2.4.4-1.7 1.7.4 2.4-2.2-1.2-2.2 1.2.4-2.4-1.7-1.7 2.4-.4Z",
        "shift": "M12 4L18 10H14.5V20H9.5V10H6Z",
        "sparkles": "m12 2 2.5 7.5L22 12l-7.5 2.5L12 22l-2.5-7.5L2 12l7.5-2.5L12 2ZM20 2v4M18 4h4",
        "speaker": "M4 9h4l5-4v14l-5-4H4V9ZM16 9c2 2 2 4 0 6M19 6c4 4 4 8 0 12",
        "sun": "M8.4 12a3.6 3.6 0 1 0 7.2 0a3.6 3.6 0 1 0 -7.2 0ZM12 3L12 5.2M12 18.8L12 21M3 12L5.2 12M18.8 12L21 12M5.64 5.64L7.2 7.2M16.8 16.8L18.36 18.36M18.36 5.64L16.8 7.2M7.2 16.8L5.64 18.36",
        "target": "M3 12a9 9 0 1 0 18 0a9 9 0 1 0 -18 0ZM7 12a5 5 0 1 0 10 0a5 5 0 1 0 -10 0ZM11 12a1 1 0 1 0 2 0a1 1 0 1 0 -2 0Z",
        "trash": "M4.5 7h15M9.5 7V4.5h5V7M6.5 7l1 12.5h9l1-12.5",
        "turtle": "M5 14c0-3.6 2.9-6.5 6.5-6.5S18 10.4 18 14H5ZM18 12.5h1.5a1.5 1.5 0 0 0 0-3H18M7 14v2.5M15.5 14v2.5M8.5 10.2 11.5 14l3-3.8",
        "upload": "M12 15V4M7.5 8.5L12 4l4.5 4.5M5 19.5h14",
        "users": "M5.8 8a3.2 3.2 0 1 0 6.4 0a3.2 3.2 0 1 0 -6.4 0ZM3.5 19.5c.6-3.2 2.8-5 5.5-5s4.9 1.8 5.5 5M13.9 9a2.6 2.6 0 1 0 5.2 0a2.6 2.6 0 1 0 -5.2 0ZM15.5 14.6c2.4-.2 4.4 1.3 5 4.4",
        "waveform": "M2 10v4M6 7v10M10 3v18M14 8v8M18 5v14M22 10v4",
        "x": "M6 6l12 12M18 6 6 18"
    })

    // The sources use a 24-unit grid with a 1.8 stroke; scaling the shape scales the stroke with it.
    Shape {
        width: 24
        height: 24
        scale: icon.size / 24
        transformOrigin: Item.TopLeft
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: icon.color
            strokeWidth: 1.8
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathSvg {
                path: icon.paths[icon.name] ?? ""
            }
        }
    }
}
