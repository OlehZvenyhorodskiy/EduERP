import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: leaderboardView
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent; spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Text { text: "🏆 Ranglijst"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
            Item { Layout.fillWidth: true }
            Repeater {
                model: ["Deze week", "Deze maand", "Altijd"]
                delegate: Rectangle {
                    width: periodText.implicitWidth + 20; height: 32; radius: 16
                    color: index === 1 ? "#2563EB" : "#F1F5F9"
                    Text { id: periodText; anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 1 ? "#FFF" : "#475569" }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                }
            }
        }

        // Podium — top 3
        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 200; spacing: 16
            Layout.alignment: Qt.AlignHCenter

            Repeater {
                model: ListModel {
                    ListElement { rank: "🥈"; team: "—"; profit: "€ 0"; placement: 2; podiumH: 120 }
                    ListElement { rank: "🥇"; team: "—"; profit: "€ 0"; placement: 1; podiumH: 160 }
                    ListElement { rank: "🥉"; team: "—"; profit: "€ 0"; placement: 3; podiumH: 90 }
                }

                delegate: ColumnLayout {
                    Layout.fillWidth: true; spacing: 8; Layout.alignment: Qt.AlignBottom

                    Text { text: model.rank; font.pixelSize: 32; Layout.alignment: Qt.AlignHCenter }
                    Text { text: model.team; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B"; Layout.alignment: Qt.AlignHCenter }
                    Text { text: model.profit; font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter }

                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: model.podiumH
                        radius: 8
                        gradient: Gradient {
                            GradientStop { position: 0.0; color: model.placement === 1 ? "#FCD34D" : (model.placement === 2 ? "#E2E8F0" : "#D97706") }
                            GradientStop { position: 1.0; color: model.placement === 1 ? "#F59E0B" : (model.placement === 2 ? "#CBD5E1" : "#B45309") }
                        }
                    }
                }
            }
        }

        // Full ranking table
        Rectangle {
            Layout.fillWidth: true; Layout.fillHeight: true; radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 0

                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 40; color: "#F8FAFC"; radius: 6
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                        Text { Layout.preferredWidth: 40; text: "#"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                        Text { Layout.fillWidth: true; text: "Team / Bedrijf"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                        Text { Layout.preferredWidth: 100; text: "Omzet"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                        Text { Layout.preferredWidth: 100; text: "Nettowinst"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true; Layout.fillHeight: true; color: "#FAFAFA"; radius: 6
                    Text { anchors.centerIn: parent; text: "De ranglijst verschijnt zodra meerdere teams actief zijn"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
                }
            }
        }
    }
}
