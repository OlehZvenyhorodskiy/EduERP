import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: marketingView
    color: "transparent"

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: marketingView.width; spacing: 20

            Text { text: "📢 Marketing"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }

            RowLayout {
                Layout.fillWidth: true; spacing: 16

                Repeater {
                    model: ListModel {
                        ListElement { title: "Naamsbekendheid"; value: "5%"; icon: "🎯" }
                        ListElement { title: "Actieve campagnes"; value: "0"; icon: "📣" }
                        ListElement { title: "Marketing budget"; value: "€ 0,00"; icon: "💰" }
                        ListElement { title: "Klantacquisitie"; value: "0/dag"; icon: "📊" }
                    }
                    delegate: Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 90
                        radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 14; spacing: 6
                            Text { text: model.icon + " " + model.title; font.pixelSize: 12; font.family: "Inter"; color: "#64748B" }
                            Text { text: model.value; font.pixelSize: 24; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
                        }
                    }
                }
            }

            // Campaigns
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 400
                radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "📣 Campagnes"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 160; height: 32; radius: 6; color: "#2563EB"
                            Text { anchors.centerIn: parent; text: "+ Nieuwe campagne"; font.pixelSize: 13; font.family: "Inter"; color: "#FFF" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }

                    // Campaign Types
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8

                        Repeater {
                            model: ["Social Media", "Print", "Evenement", "E-mail", "Sponsoring"]
                            delegate: Rectangle {
                                Layout.preferredHeight: 28
                                Layout.preferredWidth: chipText.implicitWidth + 20
                                radius: 14; color: "#F1F5F9"
                                Text { id: chipText; anchors.centerIn: parent; text: modelData; font.pixelSize: 12; font.family: "Inter"; color: "#475569" }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; color: "#FAFAFA"; radius: 6
                        Text { anchors.centerIn: parent; text: "Start je eerste marketingcampagne!"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
                    }
                }
            }
        }
    }
}
