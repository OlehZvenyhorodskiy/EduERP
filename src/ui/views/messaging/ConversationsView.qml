import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: conversationsView
    color: "transparent"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Conversations List
        Rectangle {
            Layout.preferredWidth: 320
            Layout.fillHeight: true
            color: "#FFFFFF"
            border.color: "#E2E8F0"

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Header
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 56
                    color: "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16
                        Text { text: "💬 Berichten"; font.pixelSize: 18; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 32; height: 32; radius: 16; color: "#2563EB"
                            Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 18; color: "#FFF" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }
                }

                // Search
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 40
                    Layout.leftMargin: 12; Layout.rightMargin: 12
                    radius: 8; color: "#F1F5F9"
                    Text { anchors.centerIn: parent; text: "🔍 Zoeken..."; font.pixelSize: 13; font.family: "Inter"; color: "#94A3B8" }
                }

                Item { Layout.preferredHeight: 8 }

                // Empty state
                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; color: "transparent"
                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 8
                        Text { text: "💬"; font.pixelSize: 40; Layout.alignment: Qt.AlignHCenter }
                        Text { text: "Geen gesprekken"; font.pixelSize: 14; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter }
                        Text { text: "Start een nieuw gesprek"; font.pixelSize: 12; font.family: "Inter"; color: "#94A3B8"; Layout.alignment: Qt.AlignHCenter }
                    }
                }
            }
        }

        // Chat Area
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true
            color: "#F8FAFC"

            ColumnLayout {
                anchors.centerIn: parent; spacing: 8
                Text { text: "💬"; font.pixelSize: 48; Layout.alignment: Qt.AlignHCenter }
                Text { text: "Selecteer een gesprek"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.Medium; color: "#64748B"; Layout.alignment: Qt.AlignHCenter }
                Text { text: "Kies een gesprek aan de linkerkant om te beginnen"; font.pixelSize: 13; font.family: "Inter"; color: "#94A3B8"; Layout.alignment: Qt.AlignHCenter }
            }
        }
    }
}
