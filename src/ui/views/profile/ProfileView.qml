import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: profileView
    color: "transparent"

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: profileView.width; spacing: 0

            // Banner
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 200
                color: "#1E40AF"
                gradient: Gradient {
                    GradientStop { position: 0.0; color: "#1E40AF" }
                    GradientStop { position: 1.0; color: "#7C3AED" }
                }

                // Avatar overlay
                Rectangle {
                    anchors.bottom: parent.bottom; anchors.bottomMargin: -40
                    anchors.left: parent.left; anchors.leftMargin: 32
                    width: 96; height: 96; radius: 48
                    color: "#2563EB"; border.color: "#FFFFFF"; border.width: 4

                    Text { anchors.centerIn: parent; text: "J"; font.pixelSize: 36; font.family: "Inter"; font.weight: Font.Bold; color: "#FFFFFF" }
                }
            }

            Item { Layout.preferredHeight: 56 }

            // Profile Info
            ColumnLayout {
                Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; spacing: 4

                RowLayout {
                    spacing: 12
                    Text { text: "Jan De Smet"; font.pixelSize: 24; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B" }
                    Rectangle {
                        width: roleText.implicitWidth + 12; height: 22; radius: 11
                        color: "#EFF6FF"
                        Text { id: roleText; anchors.centerIn: parent; text: "Student"; font.pixelSize: 11; font.family: "Inter"; font.weight: Font.Medium; color: "#2563EB" }
                    }
                }
                Text { text: "@jan.desmet • 5de jaar Economie A"; font.pixelSize: 14; font.family: "Inter"; color: "#64748B" }
                Text { text: "Geïnteresseerd in ondernemerschap 🚀"; font.pixelSize: 14; font.family: "Inter"; color: "#475569"; Layout.topMargin: 8 }
            }

            Item { Layout.preferredHeight: 24 }

            // Stats
            RowLayout {
                Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; spacing: 16

                Repeater {
                    model: ListModel {
                        ListElement { label: "Bedrijven"; value: "0" }
                        ListElement { label: "Simulatie-uren"; value: "0" }
                        ListElement { label: "Beste winst"; value: "€ 0" }
                        ListElement { label: "Vrienden"; value: "0" }
                    }
                    delegate: Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 80
                        radius: 10; color: "#FFFFFF"; border.color: "#E2E8F0"
                        ColumnLayout {
                            anchors.centerIn: parent; spacing: 4
                            Text { text: model.value; font.pixelSize: 22; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"; Layout.alignment: Qt.AlignHCenter }
                            Text { text: model.label; font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter }
                        }
                    }
                }
            }
        }
    }
}
