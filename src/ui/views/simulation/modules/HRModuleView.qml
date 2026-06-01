import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: hrView
    color: "transparent"

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: hrView.width; spacing: 20

            Text { text: "👥 Personeel (HR)"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }

            RowLayout {
                Layout.fillWidth: true; spacing: 16

                Repeater {
                    model: ListModel {
                        ListElement { title: "Werknemers"; value: "0"; icon: "👤" }
                        ListElement { title: "Maandloon totaal"; value: "€ 0,00"; icon: "💵" }
                        ListElement { title: "Gem. tevredenheid"; value: "—"; icon: "😊" }
                        ListElement { title: "Gem. prestatie"; value: "—"; icon: "📈" }
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

            // Employee List
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 400
                radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "📋 Personeelsbestand"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 150; height: 32; radius: 6; color: "#2563EB"
                            Text { anchors.centerIn: parent; text: "+ Werven"; font.pixelSize: 13; font.family: "Inter"; font.weight: Font.Medium; color: "#FFF" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 36; color: "#F8FAFC"; radius: 6
                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12
                            Text { Layout.fillWidth: true; text: "Naam"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.preferredWidth: 100; text: "Afdeling"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.preferredWidth: 90; text: "Salaris"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                            Text { Layout.preferredWidth: 80; text: "Tevredenheid"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.fillHeight: true; color: "#FAFAFA"; radius: 6
                        Text { anchors.centerIn: parent; text: "Neem je eerste werknemer aan!"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8" }
                    }
                }
            }
        }
    }
}
