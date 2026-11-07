import QtQuick

// A Spanish word or phrase; for nouns, the article takes its gender colour (blue el, rose la).
Text {
    id: word

    property string spanish
    property string gender  // "m", "f" or empty

    function escaped(s) {
        return s.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
    }

    readonly property int space: spanish.indexOf(" ")

    text: gender !== "" && space > 0
          ? "<font color=\"" + (gender === "m" ? Theme.masculine : Theme.feminine) + "\">"
            + escaped(spanish.slice(0, space)) + "</font>" + escaped(spanish.slice(space))
          : escaped(spanish)
    textFormat: Text.StyledText
    color: Theme.ink
    font.pixelSize: 20
    font.weight: Font.DemiBold
    wrapMode: Text.WordWrap
}
