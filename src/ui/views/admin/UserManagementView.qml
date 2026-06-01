import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: userMgmtView
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent; spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Text { text: "👥 Gebruikersbeheer"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
            Item { Layout.fillWidth: true }

            Rectangle {
                width: 140; height: 36; radius: 8; color: "#2563EB"
                Text { anchors.centerIn: parent; text: "📥 Importeren"; font.pixelSize: 13; font.family: "Inter"; color: "#FFF" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
            }

            Rectangle {
                width: 140; height: 36; radius: 8; color: "#16A34A"
                Text { anchors.centerIn: parent; text: "+ Gebruiker"; font.pixelSize: 13; font.family: "Inter"; color: "#FFF" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
            }
        }

        // Filters
        RowLayout {
            Layout.fillWidth: true; spacing: 12

            Rectangle {
                Layout.fillWidth: true; Layout.maximumWidth: 300; height: 36; radius: 8; color: "#F1F5F9"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 10; spacing: 8
                    Text { text: "🔍"; font.pixelSize: 14 }
                    Text { text: "Zoek gebruikers..."; font.pixelSize: 13; font.family: "Inter"; color: "#94A3B8" }
                }
            }

            Repeater {
                model: ["Alle", "Student", "Leraar", "Admin"]
                delegate: Rectangle {
                    width: filterText.implicitWidth + 20; height: 32; radius: 16
                    color: index === 0 ? "#2563EB" : "#F1F5F9"
                    Text { id: filterText; anchors.centerIn: parent; text: modelData; font.pixelSize: 12; font.family: "Inter"; color: index === 0 ? "#FFF" : "#475569" }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                }
            }
        }

        // User table
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 0

                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 40; color: "#F8FAFC"; radius: 6
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                        Text { Layout.fillWidth: true; text: "Naam"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                        Text { Layout.preferredWidth: 180; text: "E-mail"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                        Text { Layout.preferredWidth: 80; text: "Rol"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                        Text { Layout.preferredWidth: 80; text: "Status"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                        Text { Layout.preferredWidth: 120; text: "Laatste login"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; color: "#FAFAFA"; radius: 6
                    Text { anchors.centerIn: parent; text: "Geen gebruikers gevonden"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
                }
            }
        }
    }
}
