import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: salesView
    color: "transparent"

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: salesView.width
            spacing: 20

            Text { text: "🛒 Verkoop & CRM"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }

            // Pipeline Stages
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Repeater {
                    model: ListModel {
                        ListElement { stage: "Leads"; count: "0"; color: "#3B82F6" }
                        ListElement { stage: "Prospects"; count: "0"; color: "#8B5CF6" }
                        ListElement { stage: "Voorstel"; count: "0"; color: "#F59E0B" }
                        ListElement { stage: "Onderhandeling"; count: "0"; color: "#EF4444" }
                        ListElement { stage: "Gewonnen"; count: "0"; color: "#10B981" }
                    }

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 90
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#E2E8F0"

                        Rectangle {
                            width: parent.width; height: 3
                            anchors.top: parent.top
                            radius: 10
                            color: model.color
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 14
                            spacing: 6
                            Text { text: model.stage; font.pixelSize: 12; font.family: "Inter"; color: "#64748B" }
                            Text { text: model.count; font.pixelSize: 28; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
                        }
                    }
                }
            }

            // Customers Table
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 350
                radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "👤 Klanten"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 130; height: 32; radius: 6; color: "#2563EB"
                            Text { anchors.centerIn: parent; text: "+ Nieuwe klant"; font.pixelSize: 13; font.family: "Inter"; color: "#FFF" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }

                    // Table Header
                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 36
                        color: "#F8FAFC"; radius: 6

                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                            Text { Layout.fillWidth: true; text: "Naam"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.preferredWidth: 80; text: "Type"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.preferredWidth: 80; text: "Loyaliteit"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                            Text { Layout.preferredWidth: 100; text: "Totale waarde"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; color: "#FAFAFA"; radius: 6
                        Text { anchors.centerIn: parent; text: "Klanten verschijnen zodra de simulatie start"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
                    }
                }
            }
        }
    }
}
