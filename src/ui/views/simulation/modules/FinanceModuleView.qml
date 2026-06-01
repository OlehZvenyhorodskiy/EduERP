import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: financeView
    color: "transparent"

    ScrollView {
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: financeView.width
            spacing: 20

            // Header
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "💰 Financiën"
                    font.pixelSize: 20
                    font.family: "Inter"
                    font.weight: Font.Bold
                    color: "#1E293B"
                }

                Item { Layout.fillWidth: true }

                // Export button
                Rectangle {
                    width: 140; height: 36; radius: 6
                    color: exportMouse.containsMouse ? "#EFF6FF" : "#FFFFFF"
                    border.color: "#2563EB"; border.width: 1
                    Text {
                        anchors.centerIn: parent
                        text: "📄 Exporteren"
                        font.pixelSize: 13; font.family: "Inter"
                        color: "#2563EB"
                    }
                    MouseArea { id: exportMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
                }
            }

            // Financial Summary Cards
            RowLayout {
                Layout.fillWidth: true
                spacing: 16

                Repeater {
                    model: ListModel {
                        ListElement { title: "Omzet"; value: "€ 0,00"; trend: "↗ +0%"; trendColor: "#16A34A" }
                        ListElement { title: "Kosten"; value: "€ 0,00"; trend: "→ Stabiel"; trendColor: "#D97706" }
                        ListElement { title: "Nettowinst"; value: "€ 0,00"; trend: "→ Stabiel"; trendColor: "#D97706" }
                        ListElement { title: "Kas"; value: "€ 100.000"; trend: "Startkapitaal"; trendColor: "#64748B" }
                    }

                    delegate: Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 100
                        radius: 10
                        color: "#FFFFFF"
                        border.color: "#E2E8F0"

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 16
                            spacing: 6

                            Text { text: model.title; font.pixelSize: 12; font.family: "Inter"; color: "#64748B" }
                            Text { text: model.value; font.pixelSize: 22; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
                            Text { text: model.trend; font.pixelSize: 11; font.family: "Inter"; color: model.trendColor }
                        }
                    }
                }
            }

            // Ledger / Journal Table
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 400
                radius: 10
                color: "#FFFFFF"
                border.color: "#E2E8F0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "📒 Grootboek"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                        Item { Layout.fillWidth: true }
                        Rectangle {
                            width: 120; height: 32; radius: 6
                            color: "#2563EB"
                            Text { anchors.centerIn: parent; text: "+ Boeking"; font.pixelSize: 13; font.family: "Inter"; color: "#FFFFFF" }
                            MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                        }
                    }

                    // Table Header
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 36
                        color: "#F8FAFC"
                        radius: 6

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12

                            Text { Layout.preferredWidth: 100; text: "Datum"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.fillWidth: true; text: "Omschrijving"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B" }
                            Text { Layout.preferredWidth: 100; text: "Debet"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                            Text { Layout.preferredWidth: 100; text: "Credit"; font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold; color: "#64748B"; horizontalAlignment: Text.AlignRight }
                        }
                    }

                    // Empty state
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#FAFAFA"
                        radius: 6

                        Text {
                            anchors.centerIn: parent
                            text: "Nog geen boekingen. Maak je eerste transactie!"
                            font.pixelSize: 14; font.family: "Inter"
                            color: "#94A3B8"
                        }
                    }
                }
            }
        }
    }
}
