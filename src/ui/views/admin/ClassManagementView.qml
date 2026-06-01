import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: classMgmtView
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent; spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Text { text: "🏫 Klassenbeheer"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: 140; height: 36; radius: 8; color: "#2563EB"
                Text { anchors.centerIn: parent; text: "+ Nieuwe klas"; font.pixelSize: 13; font.family: "Inter"; color: "#FFF" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
            }
        }

        // Class cards grid
        GridLayout {
            Layout.fillWidth: true; columns: 2; rowSpacing: 16; columnSpacing: 16

            Repeater {
                model: ListModel {
                    ListElement { className: "5de jaar Economie A"; teacher: "Mevr. De Vos"; students: "0"; teams: "0"; year: "2025-2026" }
                }

                delegate: Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 140
                    radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"

                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 20; spacing: 10

                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: model.className; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                            Item { Layout.fillWidth: true }
                            Rectangle {
                                width: yearText.implicitWidth + 12; height: 22; radius: 11; color: "#F1F5F9"
                                Text { id: yearText; anchors.centerIn: parent; text: model.year; font.pixelSize: 11; font.family: "Inter"; color: "#475569" }
                            }
                        }

                        Text { text: "Leraar: " + model.teacher; font.pixelSize: 13; font.family: "Inter"; color: "#64748B" }

                        RowLayout {
                            spacing: 16
                            Text { text: "👥 " + model.students + " leerlingen"; font.pixelSize: 12; font.family: "Inter"; color: "#94A3B8" }
                            Text { text: "🏢 " + model.teams + " teams"; font.pixelSize: 12; font.family: "Inter"; color: "#94A3B8" }
                        }
                    }

                    MouseArea { anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
                }
            }
        }

        // Empty state for remaining space
        Item { Layout.fillHeight: true }
    }
}
