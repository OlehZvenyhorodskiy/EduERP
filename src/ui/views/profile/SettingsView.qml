import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: settingsView
    color: "transparent"

    // Reactive: when translator.currentLocale changes, all t() bindings re-evaluate
    property string locale: translator ? translator.currentLocale : "nl-BE"

    ScrollView {
        anchors.fill: parent; clip: true

        ColumnLayout {
            width: settingsView.width; spacing: 24

            Text {
                text: "⚙️ " + (translator ? translator.t("settings.title") : "Instellingen")
                font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"
            }

            // ── Appearance Section ──
            Rectangle {
                Layout.fillWidth: true; radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"
                Layout.preferredHeight: appCol.implicitHeight + 40

                ColumnLayout {
                    id: appCol
                    anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                    anchors.margins: 20; spacing: 16

                    Text {
                        text: "🎨 " + (translator ? translator.t("settings.appearance") : "Uiterlijk")
                        font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B"
                    }

                    // Theme
                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
                        Text {
                            text: translator ? translator.t("settings.theme") : "Thema"
                            font.pixelSize: 14; font.family: "Inter"; color: "#475569"; Layout.preferredWidth: 140
                        }
                        Repeater {
                            model: [
                                translator ? translator.t("settings.theme_system") : "Systeem",
                                translator ? translator.t("settings.theme_light") : "Licht",
                                translator ? translator.t("settings.theme_dark") : "Donker"
                            ]
                            delegate: Rectangle {
                                width: 80; height: 32; radius: 6
                                color: index === 0 ? "#2563EB" : "#F1F5F9"
                                Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 0 ? "#FFF" : "#475569" }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }

                    // Font Size
                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
                        Text {
                            text: translator ? translator.t("settings.font_size") : "Lettergrootte"
                            font.pixelSize: 14; font.family: "Inter"; color: "#475569"; Layout.preferredWidth: 140
                        }
                        Repeater {
                            model: [
                                translator ? translator.t("settings.font_small") : "Klein",
                                translator ? translator.t("settings.font_normal") : "Normaal",
                                translator ? translator.t("settings.font_large") : "Groot"
                            ]
                            delegate: Rectangle {
                                width: 80; height: 32; radius: 6
                                color: index === 1 ? "#2563EB" : "#F1F5F9"
                                Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 1 ? "#FFF" : "#475569" }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }

                    // Animations
                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
                        Text {
                            text: translator ? translator.t("settings.animations") : "Animaties"
                            font.pixelSize: 14; font.family: "Inter"; color: "#475569"; Layout.preferredWidth: 140
                        }
                        Repeater {
                            model: [
                                translator ? translator.t("settings.anim_full") : "Volledig",
                                translator ? translator.t("settings.anim_reduced") : "Verminderd",
                                translator ? translator.t("settings.anim_none") : "Geen"
                            ]
                            delegate: Rectangle {
                                width: 90; height: 32; radius: 6
                                color: index === 0 ? "#2563EB" : "#F1F5F9"
                                Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 0 ? "#FFF" : "#475569" }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }
                }
            }

            // ── Language Section ──
            Rectangle {
                Layout.fillWidth: true; radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"
                Layout.preferredHeight: langCol.implicitHeight + 40

                ColumnLayout {
                    id: langCol
                    anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                    anchors.margins: 20; spacing: 16

                    Text {
                        text: "🌍 " + (translator ? translator.t("settings.language") : "Taal")
                        font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B"
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 12

                        Repeater {
                            model: translator ? translator.availableLocales : ["nl-BE", "en-GB", "fr-BE"]

                            delegate: Rectangle {
                                width: 180; height: 48; radius: 8
                                color: modelData === locale ? "#EFF6FF" : "#F8FAFC"
                                border.color: modelData === locale ? "#2563EB" : "#E2E8F0"
                                border.width: modelData === locale ? 2 : 1

                                Behavior on border.color { ColorAnimation { duration: 200 } }

                                RowLayout {
                                    anchors.centerIn: parent; spacing: 10
                                    Text {
                                        text: translator ? translator.localeFlag(modelData) : "🏳️"
                                        font.pixelSize: 22
                                    }
                                    ColumnLayout {
                                        spacing: 0
                                        Text {
                                            text: translator ? translator.localeName(modelData) : modelData
                                            font.pixelSize: 13; font.family: "Inter"
                                            font.weight: modelData === locale ? Font.DemiBold : Font.Normal
                                            color: "#1E293B"
                                        }
                                        Text {
                                            text: modelData
                                            font.pixelSize: 10; font.family: "Inter"; color: "#94A3B8"
                                        }
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent; cursorShape: Qt.PointingHandCursor
                                    onClicked: {
                                        if (translator) {
                                            translator.setLocale(modelData)
                                            // Force QML re-evaluation
                                            settingsView.locale = modelData
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ── Privacy Section ──
            Rectangle {
                Layout.fillWidth: true; radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"
                Layout.preferredHeight: privCol.implicitHeight + 40

                ColumnLayout {
                    id: privCol
                    anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
                    anchors.margins: 20; spacing: 16

                    Text {
                        text: "🔒 " + (translator ? translator.t("settings.privacy") : "Privacy")
                        font.pixelSize: 16; font.family: "Inter"; font.weight: Font.DemiBold; color: "#1E293B"
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
                        Text {
                            text: translator ? translator.t("settings.visibility") : "Profiel zichtbaar voor"
                            font.pixelSize: 14; font.family: "Inter"; color: "#475569"; Layout.preferredWidth: 160
                        }
                        Repeater {
                            model: [
                                translator ? translator.t("settings.visibility_everyone") : "Iedereen",
                                translator ? translator.t("settings.visibility_friends") : "Vrienden",
                                translator ? translator.t("settings.visibility_class") : "Klas",
                                translator ? translator.t("settings.visibility_teachers") : "Leraren"
                            ]
                            delegate: Rectangle {
                                width: 90; height: 32; radius: 6
                                color: index === 1 ? "#2563EB" : "#F1F5F9"
                                Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 1 ? "#FFF" : "#475569" }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true; spacing: 12
                        Text {
                            text: translator ? translator.t("settings.friend_requests") : "Vriendschapsverzoeken"
                            font.pixelSize: 14; font.family: "Inter"; color: "#475569"; Layout.preferredWidth: 160
                        }
                        Repeater {
                            model: [
                                translator ? translator.t("settings.fr_everyone") : "Iedereen",
                                translator ? translator.t("settings.fr_class") : "Klas",
                                translator ? translator.t("settings.fr_disabled") : "Uitgeschakeld"
                            ]
                            delegate: Rectangle {
                                width: 110; height: 32; radius: 6
                                color: index === 1 ? "#2563EB" : "#F1F5F9"
                                Text { anchors.centerIn: parent; text: modelData; font.pixelSize: 13; font.family: "Inter"; color: index === 1 ? "#FFF" : "#475569" }
                                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                            }
                        }
                    }
                }
            }

            // ── Energy Saving ──
            Rectangle {
                Layout.fillWidth: true; radius: 12; color: "#FFFFFF"; border.color: "#E2E8F0"
                Layout.preferredHeight: 80

                RowLayout {
                    anchors.fill: parent; anchors.margins: 20; spacing: 16
                    Text {
                        text: "🔋 " + (translator ? translator.t("settings.energy_saving") : "Energiebesparende modus")
                        font.pixelSize: 14; font.family: "Inter"; font.weight: Font.Medium; color: "#1E293B"; Layout.fillWidth: true
                    }
                    Rectangle {
                        width: 48; height: 26; radius: 13
                        color: "#E2E8F0"
                        Rectangle {
                            width: 22; height: 22; radius: 11; x: 2; anchors.verticalCenter: parent.verticalCenter
                            color: "#FFFFFF"
                        }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                    }
                }
            }
        }
    }
}
