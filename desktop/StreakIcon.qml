import QtQuick
import QtQuick.Shapes

// The streak's icon: the emoji the learner picked, or else the app's own two-tone flame (the paths of
// images/streak.svg, drawn as shapes so it stays sharp at any size). Faded when there is no streak to show.
Item {
    id: flame

    property string emoji
    property real size: 24
    property bool lit: true

    implicitWidth: size
    implicitHeight: size

    Text {
        visible: flame.emoji !== ""
        anchors.centerIn: parent
        text: flame.emoji
        opacity: flame.lit ? 1 : 0.4
        font.pixelSize: Math.round(flame.size * 0.85)
        textFormat: Text.PlainText
    }

    Shape {
        visible: flame.emoji === ""
        width: 64
        height: 64
        scale: flame.size / 64
        transformOrigin: Item.TopLeft
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: "transparent"
            fillGradient: LinearGradient {
                y1: 3
                y2: 61
                GradientStop { position: 0; color: flame.lit ? "#ffa043" : Theme.dark ? "#5c4f48" : "#d9d0cb" }
                GradientStop { position: 1; color: flame.lit ? "#e8501b" : Theme.dark ? "#463a34" : "#b2a7a1" }
            }
            PathSvg {
                path: "M33 3C35 13 44 19 48 27C52 34 53 41 51 47C48 56 41 61 32 61C22 61 13 54 13 43C13 35 17 28 22 23"
                      + "C22 29 24 33 27 35C26 24 28 12 33 3Z"
            }
        }
        ShapePath {
            strokeColor: "transparent"
            fillGradient: LinearGradient {
                y1: 29
                y2: 59
                GradientStop { position: 0; color: flame.lit ? "#ffe08a" : Theme.dark ? "#73655e" : "#f1ece9" }
                GradientStop { position: 1; color: flame.lit ? "#ffb43a" : Theme.dark ? "#5c4f48" : "#ddd5d0" }
            }
            PathSvg {
                path: "M32 29C34 36 42 40 42 49C42 55 37 59 32 59C27 59 22 55 22 49C22 44 25 41 27 39C27 43 29 45 30 45"
                      + "C29 40 29 34 32 29Z"
            }
        }
    }
}
