import QtQuick
import QtQuick.Layouts

/**
 * Compact streak widget for embedding in the dashboard or sidebar.
 * Shows flame icon, current streak count, and a mini calendar row.
 */
Rectangle {
    id: streakWidget
    width: parent ? parent.width : 300
    height: 64
    radius: 10
    color: "#FFFFFF"
    border.color: "#E2E8F0"

    property int streak: gamificationController ? gamificationController.currentStreak : 0
    property int level: gamificationController ? gamificationController.currentLevel : 1
    property double progress: gamificationController ? gamificationController.levelProgress : 0

    RowLayout {
        anchors.fill: parent; anchors.margins: 12; spacing: 12

        // Flame with streak count
        Rectangle {
            width: 42; height: 42; radius: 21
            color: streak > 0 ? "#FEF3C7" : "#F1F5F9"
            Text {
                anchors.centerIn: parent
                text: streak > 0 ? "🔥" : "❄️"
                font.pixelSize: 22
            }
        }

        ColumnLayout {
            Layout.fillWidth: true; spacing: 2
            Text {
                text: streak + " " + (translator ? translator.t("gamification.day_streak") : "dagen streak")
                font.pixelSize: 14; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B"
            }
            Text {
                text: "Lv." + level + " • " + Math.round(progress * 100) + "% " + (translator ? translator.t("common.next") : "volgende")
                font.pixelSize: 11; font.family: "Inter"; color: "#64748B"
            }
        }

        // Mini 7-day calendar dots
        RowLayout {
            spacing: 3
            Repeater {
                model: 7
                delegate: Rectangle {
                    width: 8; height: 8; radius: 4
                    // Light up dots for days within the streak (right-to-left, today = rightmost)
                    color: (6 - index) < streak ? "#16A34A" : "#E2E8F0"
                    Behavior on color { ColorAnimation { duration: 200 } }
                }
            }
        }
    }
}
