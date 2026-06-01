import QtQuick
import QtQuick.Layouts

/**
 * Toast popup for newly unlocked achievements.
 * Slides in from top-right corner.
 */
Rectangle {
    id: achievementToast

    property string achievementIcon: "🏆"
    property string achievementTitle: ""
    property bool showing: false

    width: 300; height: 72; radius: 12
    anchors.right: parent ? parent.right : undefined
    anchors.rightMargin: 20
    y: showing ? 20 : -100

    Behavior on y { NumberAnimation { duration: 400; easing.type: Easing.OutBack } }

    color: "#FFFFFF"
    border.color: "#BBF7D0"
    border.width: 2

    RowLayout {
        anchors.fill: parent; anchors.margins: 14; spacing: 12

        Text { text: achievementIcon; font.pixelSize: 28 }

        ColumnLayout {
            Layout.fillWidth: true; spacing: 2
            Text {
                text: translator ? translator.t("gamification.achievement_unlocked") : "Achievement unlocked!"
                font.pixelSize: 11; font.family: "Inter"; font.weight: Font.DemiBold; color: "#16A34A"
                font.letterSpacing: 1
            }
            Text {
                text: achievementTitle
                font.pixelSize: 14; font.family: "Inter"; font.weight: Font.Medium; color: "#1E293B"
                elide: Text.ElideRight; Layout.fillWidth: true
            }
        }
    }

    Timer {
        running: showing; interval: 5000
        onTriggered: showing = false
    }

    function show(icon, title) {
        achievementIcon = icon
        achievementTitle = title
        showing = true
    }
}
