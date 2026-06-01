import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: companyListView
    color: "transparent"

    ColumnLayout {
        anchors.fill: parent; spacing: 20

        RowLayout {
            Layout.fillWidth: true
            Text { text: "🏢 Mijn Bedrijven"; font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
            Item { Layout.fillWidth: true }
            Rectangle {
                width: 160; height: 36; radius: 8; color: "#2563EB"
                Text { anchors.centerIn: parent; text: "+ Nieuw bedrijf"; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.Medium; color: "#FFF" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }

                scale: createMouse.pressed ? 0.95 : (createMouse.containsMouse ? 1.02 : 1.0)
                Behavior on scale { NumberAnimation { duration: 150; easing.type: Easing.OutQuad } }
                MouseArea { id: createMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
            }
        }

        // Industry Template Selection
        Text { text: "Kies een branche:"; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.Medium; color: "#475569" }

        GridLayout {
            Layout.fillWidth: true; columns: 3; rowSpacing: 12; columnSpacing: 12

            Repeater {
                model: ListModel {
                    ListElement { industry: "Kledingwinkel"; description: "Detailhandel in mode"; icon: "👔"; template: "retail_clothing" }
                    ListElement { industry: "Tech Hardware"; description: "Computerapparatuur"; icon: "💻"; template: "tech_hardware" }
                    ListElement { industry: "E-commerce"; description: "Online marktplaats"; icon: "🛍️"; template: "ecommerce_marketplace" }
                    ListElement { industry: "Logistiek"; description: "Bezorg- en transportdiensten"; icon: "🚚"; template: "logistics_delivery" }
                    ListElement { industry: "Horeca"; description: "Eten en drinken"; icon: "🍽️"; template: "food_beverage" }
                    ListElement { industry: "Belgisch KMO"; description: "Klein/middelgroot bedrijf"; icon: "🇧🇪"; template: "belgian_sme" }
                    ListElement { industry: "Boekhouding"; description: "Accountancykantoor"; icon: "📊"; template: "accounting_services" }
                    ListElement { industry: "Tech Gigant"; description: "Groot technologiebedrijf"; icon: "🏭"; template: "tech_giant" }
                    ListElement { industry: "Halfgeleiders"; description: "Chipproductie"; icon: "🔬"; template: "semiconductor" }
                }

                delegate: Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 100
                    radius: 10; color: indMouse.containsMouse ? "#F0F9FF" : "#FFFFFF"
                    border.color: indMouse.containsMouse ? "#2563EB" : "#E2E8F0"
                    border.width: indMouse.containsMouse ? 2 : 1

                    Behavior on border.color { ColorAnimation { duration: 150 } }

                    RowLayout {
                        anchors.fill: parent; anchors.margins: 16; spacing: 14

                        Rectangle {
                            width: 48; height: 48; radius: 12; color: "#F1F5F9"
                            Text { anchors.centerIn: parent; text: model.icon; font.pixelSize: 24 }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 4
                            Text { text: model.industry; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B" }
                            Text { text: model.description; font.pixelSize: 12; font.family: "Inter"; color: "#64748B" }
                        }
                    }

                    MouseArea { id: indMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
                }
            }
        }
    }
}
