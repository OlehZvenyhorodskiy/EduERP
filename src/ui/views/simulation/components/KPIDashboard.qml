import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: kpiDashboard
    color: "transparent"

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: kpiDashboard.width; spacing: 20

            Text { text: "📊 Bedrijfsoverzicht"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }

            // Profit Gauge (visual indicator)
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 200
                radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"

                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 16

                    Text { text: "Winstgevendheid"; font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }

                    // Simple bar chart placeholder
                    RowLayout {
                        Layout.fillWidth: true; Layout.fillHeight: true; spacing: 8

                        Repeater {
                            model: ["Jan", "Feb", "Mrt", "Apr", "Mei", "Jun"]
                            delegate: ColumnLayout {
                                Layout.fillWidth: true; Layout.fillHeight: true; spacing: 4

                                Item { Layout.fillHeight: true }

                                Rectangle {
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: Math.random() * 80 + 20
                                    radius: 4
                                    color: "#2563EB"
                                    opacity: 0.3 + Math.random() * 0.7

                                    Behavior on height { NumberAnimation { duration: 500; easing.type: Easing.OutQuad } }
                                }

                                Text {
                                    text: modelData
                                    font.pixelSize: 11; font.family: "Inter"
                                    color: "#94A3B8"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                            }
                        }
                    }
                }
            }

            // Module Status Grid
            GridLayout {
                Layout.fillWidth: true
                columns: 3
                rowSpacing: 16; columnSpacing: 16

                Repeater {
                    model: ListModel {
                        ListElement { modName: "Financiën"; status: "Actief"; statusColor: "#16A34A"; icon: "💰" }
                        ListElement { modName: "Verkoop"; status: "Actief"; statusColor: "#16A34A"; icon: "🛒" }
                        ListElement { modName: "Voorraad"; status: "Actief"; statusColor: "#16A34A"; icon: "📦" }
                        ListElement { modName: "Personeel"; status: "Niet actief"; statusColor: "#94A3B8"; icon: "👥" }
                        ListElement { modName: "Marketing"; status: "Niet actief"; statusColor: "#94A3B8"; icon: "📢" }
                        ListElement { modName: "Logistiek"; status: "Vergrendeld"; statusColor: "#D97706"; icon: "🚛" }
                    }

                    delegate: Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 100
                        radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"

                        RowLayout {
                            anchors.fill: parent; anchors.margins: 16; spacing: 14

                            Rectangle {
                                width: 44; height: 44; radius: 10
                                color: "#F1F5F9"
                                Text { anchors.centerIn: parent; text: model.icon; font.pixelSize: 22 }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 4
                                Text { text: model.modName; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                                Text {
                                    text: "● " + model.status
                                    font.pixelSize: 12; font.family: "Inter"
                                    color: model.statusColor
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
