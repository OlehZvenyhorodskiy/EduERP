import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: eventLog
    color: "#FFFFFF"
    radius: 10
    border.color: "#E2E8F0"

    property var events: [] // Array of {title, description, timestamp, type}

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 12

        RowLayout {
            Layout.fillWidth: true
            Text { text: "📋 Gebeurtenissen"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
            Item { Layout.fillWidth: true }
            Text { text: events.length + " gebeurtenissen"; font.pixelSize: 12; font.family: "Inter"; color: "#94A3B8" }
        }

        ListView {
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true; spacing: 8
            model: events

            delegate: Rectangle {
                width: ListView.view.width; height: eventContent.implicitHeight + 16
                radius: 6; color: "#F8FAFC"; border.color: "#F1F5F9"

                RowLayout {
                    id: eventContent
                    anchors.fill: parent; anchors.margins: 8; spacing: 10

                    Rectangle {
                        width: 6; height: parent.height - 4; radius: 3
                        color: {
                            switch (modelData.type || "info") {
                                case "market_event": return "#3B82F6"
                                case "monthly_report": return "#8B5CF6"
                                case "warning": return "#F59E0B"
                                case "error": return "#EF4444"
                                default: return "#94A3B8"
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 2
                        Text { text: modelData.title || ""; font.pixelSize: 13; font.family: "Inter"; font.weight: Font.Medium; color: "#1E293B" }
                        Text { text: modelData.description || ""; font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; wrapMode: Text.WordWrap; Layout.fillWidth: true }
                    }

                    Text { text: modelData.timestamp || ""; font.pixelSize: 11; font.family: "Inter"; color: "#94A3B8" }
                }
            }

            // Empty state
            Rectangle {
                visible: events.length === 0
                anchors.centerIn: parent
                width: parent.width; height: parent.height; color: "transparent"
                Text { anchors.centerIn: parent; text: "Nog geen gebeurtenissen"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
            }
        }
    }
}
