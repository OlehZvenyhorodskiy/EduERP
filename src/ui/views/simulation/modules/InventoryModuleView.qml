import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: inventoryView
    color: "transparent"

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: inventoryView.width; spacing: 20

            Text { text: "📦 Voorraad"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }

            // Stock Stats
            RowLayout {
                Layout.fillWidth: true; spacing: 16

                Repeater {
                    model: ListModel {
                        ListElement { title: "Producten"; value: "0"; icon: "📋" }
                        ListElement { title: "Totale waarde"; value: "€ 0,00"; icon: "💎" }
                        ListElement { title: "Laag voorraad"; value: "0"; icon: "⚠️" }
                        ListElement { title: "Leveranciers"; value: "0"; icon: "🚚" }
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

            // Products Table
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 400
                radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "📋 Productcatalogus"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 140; height: 32; radius: 6; color: "#2563EB"
                            Text { anchors.centerIn: parent; text: "+ Nieuw product"; font.pixelSize: 13; font.family: "Inter"; color: "#FFF" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 36; color: "#F8FAFC"; radius: 6
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                            Text { Layout.preferredWidth: 60; text: "SKU"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.fillWidth: true; text: "Productnaam"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.preferredWidth: 80; text: "Voorraad"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                            Text { Layout.preferredWidth: 90; text: "Inkoopprijs"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                            Text { Layout.preferredWidth: 90; text: "Verkoopprijs"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; color: "#FAFAFA"; radius: 6
                        Text { anchors.centerIn: parent; text: "Voeg producten toe aan je catalogus"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
                    }
                }
            }
        }
    }
}
