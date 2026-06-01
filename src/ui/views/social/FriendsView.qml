import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: friendsView
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent; spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Text { text: "👥 Vrienden"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
            Item { Layout.fillWidth: true }

            // Tab chips
            Repeater {
                model: ["Alle vrienden", "Verzoeken", "Online"]
                delegate: Rectangle {
                    width: chipT.implicitWidth + 20; height: 32; radius: 16
                    color: index === 0 ? "#2563EB" : "#F1F5F9"
                    Text { id: chipT; anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 0 ? "#FFF" : "#475569" }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                }
            }
        }

        // Search
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 40; radius: 8; color: "#F1F5F9"
            RowLayout {
                anchors.fill: parent; anchors.margins: 10; spacing: 8
                Text { text: "🔍"; font.pixelSize: 14 }
                Text { text: "Zoek vrienden..."; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
            }
        }

        // Empty State
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"
            ColumnLayout {
                anchors.centerIn: parent; spacing: 12
                Text { text: "👥"; font.pixelSize: 48; Layout.alignment: Qt.AlignHCenter }
                Text { text: "Nog geen vrienden"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.Medium; color: "#64748B"; Layout.alignment: Qt.AlignHCenter }
                Text { text: "Voeg klasgenoten toe als vriend!"; font.pixelSize: 13; font.family: "Inter"; color: "#94A3B8"; Layout.alignment: Qt.AlignHCenter }
            }
        }
    }
}
