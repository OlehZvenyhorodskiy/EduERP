import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: simView
    color: "transparent"

    property int activeModule: 0

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Module Tab Bar
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "#FFFFFF"
            border.color: "#E2E8F0"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                spacing: 4

                Repeater {
                    model: ListModel {
                        ListElement { label: "Overzicht"; icon: "📊" }
                        ListElement { label: "Financiën"; icon: "💰" }
                        ListElement { label: "Verkoop"; icon: "🛒" }
                        ListElement { label: "Voorraad"; icon: "📦" }
                        ListElement { label: "Personeel"; icon: "👥" }
                        ListElement { label: "Marketing"; icon: "📢" }
                    }

                    delegate: Rectangle {
                        Layout.preferredHeight: 40
                        Layout.preferredWidth: tabText.implicitWidth + 48
                        radius: 8
                        color: activeModule === index ? "#EFF6FF" : (tabMouse.containsMouse ? "#F8FAFC" : "transparent")
                        border.color: activeModule === index ? "#2563EB" : "transparent"
                        border.width: activeModule === index ? 1.5 : 0

                        Behavior on color { ColorAnimation { duration: 150 } }

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 6

                            Text {
                                text: model.icon
                                font.pixelSize: 16
                            }
                            Text {
                                id: tabText
                                text: model.label
                                font.pixelSize: 13
                                font.family: "Inter"
                                font.weight: activeModule === index ? Font.DemiBold : Font.Normal
                                color: activeModule === index ? "#2563EB" : "#64748B"
                            }
                        }

                        MouseArea {
                            id: tabMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: activeModule = index
                        }
                    }
                }

                Item { Layout.fillWidth: true }

                // Simulation Controls
                RowLayout {
                    spacing: 8

                    Rectangle {
                        width: 32; height: 32; radius: 6
                        color: playMouse.containsMouse ? "#F0FDF4" : "transparent"
                        Text { anchors.centerIn: parent; text: "▶"; font.pixelSize: 16; color: "#16A34A" }
                        MouseArea { id: playMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
                    }
                    Rectangle {
                        width: 32; height: 32; radius: 6
                        color: pauseMouse.containsMouse ? "#FEF3C7" : "transparent"
                        Text { anchors.centerIn: parent; text: "⏸"; font.pixelSize: 16; color: "#D97706" }
                        MouseArea { id: pauseMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
                    }

                    Rectangle {
                        width: 80; height: 32; radius: 6
                        color: "#F1F5F9"
                        Text {
                            anchors.centerIn: parent
                            text: "1x snelheid"
                            font.pixelSize: 11
                            font.family: "Inter"
                            color: "#64748B"
                        }
                    }
                }
            }
        }

        // Module Content
        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            source: {
                switch (activeModule) {
                    case 0: return "components/KPIDashboard.qml"
                    case 1: return "modules/FinanceModuleView.qml"
                    case 2: return "modules/SalesModuleView.qml"
                    case 3: return "modules/InventoryModuleView.qml"
                    case 4: return "modules/HRModuleView.qml"
                    case 5: return "modules/MarketingModuleView.qml"
                    default: return "components/KPIDashboard.qml"
                }
            }
        }
    }
}
