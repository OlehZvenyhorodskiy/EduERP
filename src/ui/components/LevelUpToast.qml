import QtQuick
import QtQuick.Layouts

/**
 * Toast popup that appears when the user levels up.
 * Slides in from top, stays 4 seconds, slides out.
 */
Rectangle {
    id: levelUpToast

    property int newLevel: 1
    property string levelTitle: ""
    property bool showing: false

    width: 320; height: 80; radius: 14
    anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
    y: showing ? 20 : -100

    Behavior on y { NumberAnimation { duration: 400; easing.type: Easing.OutBack } }

    gradient: Gradient {
        orientation: Gradient.Horizontal
        GradientStop { position: 0.0; color: "#FCD34D" }
        GradientStop { position: 1.0; color: "#F59E0B" }
    }

    RowLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 12

        Text { text: "🎉"; font.pixelSize: 32 }

        ColumnLayout {
            Layout.fillWidth: true; spacing: 2
            Text {
                text: "LEVEL UP!"
                font.pixelSize: 16; font.family: "Inter"; font.weight: Font.Black; color: "#1E293B"
            }
            Text {
                text: "Level " + newLevel + " — " + levelTitle
                font.pixelSize: 13; font.family: "Inter"; font.weight: Font.Medium; color: "#78350F"
            }
        }
    }

    Timer {
        running: showing; interval: 4000
        onTriggered: showing = false
    }

    function show(level, title) {
        newLevel = level
        levelTitle = title
        showing = true
    }
}
