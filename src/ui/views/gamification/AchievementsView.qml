import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: achievementsView
    color: "transparent"

    property string locale: translator ? translator.currentLocale : "nl-BE"
    property string filterCategory: "all"

    ColumnLayout {
        anchors.fill: parent; spacing: 20

        // ── Header with XP bar ──
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 140; radius: 14
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#1E40AF" }
                GradientStop { position: 1.0; color: "#7C3AED" }
            }

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 24; spacing: 12

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        spacing: 4
                        Text {
                            text: gamificationController ? gamificationController.levelTitle : "🌱 Beginner"
                            font.pixelSize: 22; font.family: "Inter"; font.weight: Font.Bold; color: "#FFFFFF"
                        }
                        Text {
                            text: "Level " + (gamificationController ? gamificationController.currentLevel : 1)
                            font.pixelSize: 14; font.family: "Inter"; color: "#C7D2FE"
                        }
                    }
                    Item { Layout.fillWidth: true }
                    ColumnLayout {
                        spacing: 2
                        Text {
                            text: (gamificationController ? gamificationController.currentXP : 0) + " XP"
                            font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#FCD34D"
                            Layout.alignment: Qt.AlignRight
                        }
                        Text {
                            text: (gamificationController ? gamificationController.xpForNextLevel : 100) + " XP " + (translator ? translator.t("common.next") : "next")
                            font.pixelSize: 12; font.family: "Inter"; color: "#C7D2FE"
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }

                // XP Progress bar
                Rectangle {
                    Layout.fillWidth: true; height: 10; radius: 5; color: "#1E293B"
                    Rectangle {
                        width: parent.width * (gamificationController ? gamificationController.levelProgress : 0)
                        height: parent.height; radius: 5
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "#FCD34D" }
                            GradientStop { position: 1.0; color: "#F59E0B" }
                        }
                        Behavior on width { NumberAnimation { duration: 500; easing.type: Easing.OutQuad } }
                    }
                }
            }
        }

        // ── Streak & Stats Row ──
        RowLayout {
            Layout.fillWidth: true; spacing: 16

            // Current streak
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100; radius: 12
                color: "#FFFFFF"; border.color: "#E2E8F0"
                ColumnLayout {
                    anchors.centerIn: parent; spacing: 4
                    Text { text: "🔥"; font.pixelSize: 28; Layout.alignment: Qt.AlignHCenter }
                    Text {
                        text: (gamificationController ? gamificationController.currentStreak : 0) + " " + (translator ? translator.t("gamification.days") : "dagen")
                        font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"; Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: translator ? translator.t("gamification.current_streak") : "Huidige streak"
                        font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // Longest streak
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100; radius: 12
                color: "#FFFFFF"; border.color: "#E2E8F0"
                ColumnLayout {
                    anchors.centerIn: parent; spacing: 4
                    Text { text: "⭐"; font.pixelSize: 28; Layout.alignment: Qt.AlignHCenter }
                    Text {
                        text: (gamificationController ? gamificationController.longestStreak : 0) + " " + (translator ? translator.t("gamification.days") : "dagen")
                        font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"; Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: translator ? translator.t("gamification.longest_streak") : "Langste streak"
                        font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // Total days
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100; radius: 12
                color: "#FFFFFF"; border.color: "#E2E8F0"
                ColumnLayout {
                    anchors.centerIn: parent; spacing: 4
                    Text { text: "📅"; font.pixelSize: 28; Layout.alignment: Qt.AlignHCenter }
                    Text {
                        text: gamificationController ? gamificationController.totalLoginDays : 0
                        font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"; Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: translator ? translator.t("gamification.total_days") : "Totaal inlogdagen"
                        font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            // Achievements counter
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 100; radius: 12
                color: "#FFFFFF"; border.color: "#E2E8F0"
                ColumnLayout {
                    anchors.centerIn: parent; spacing: 4
                    Text { text: "🏆"; font.pixelSize: 28; Layout.alignment: Qt.AlignHCenter }
                    Text {
                        text: (gamificationController ? gamificationController.unlockedAchievements : 0) + "/" + (gamificationController ? gamificationController.totalAchievements : 0)
                        font.pixelSize: 20; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"; Layout.alignment: Qt.AlignHCenter
                    }
                    Text {
                        text: translator ? translator.t("gamification.achievements.title") : "Achievements"
                        font.pixelSize: 12; font.family: "Inter"; color: "#64748B"; Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
        }

        // ── Category filters ──
        RowLayout {
            Layout.fillWidth: true; spacing: 8
            Text {
                text: "🏅 " + (translator ? translator.t("gamification.badges") : "Badges")
                font.pixelSize: 18; font.family: "Inter"; font.weight: Font.Bold; color: "#1E293B"
            }
            Item { Layout.fillWidth: true }
            Repeater {
                model: [
                    {"key": "all", "label": translator ? translator.t("admin.all") : "Alle"},
                    {"key": "streak", "label": "🔥 Streak"},
                    {"key": "simulation", "label": "⚙️ Simulatie"},
                    {"key": "social", "label": "👥 Sociaal"}
                ]
                delegate: Rectangle {
                    width: catText.implicitWidth + 20; height: 30; radius: 15
                    color: filterCategory === modelData.key ? "#2563EB" : "#F1F5F9"
                    Text { id: catText; anchors.centerIn: parent; text: modelData.label; font.pixelSize: 12; font.family: "Inter"; color: filterCategory === modelData.key ? "#FFF" : "#475569" }
                    MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: filterCategory = modelData.key }
                }
            }
        }

        // ── Achievement Grid ──
        GridView {
            Layout.fillWidth: true; Layout.fillHeight: true
            clip: true; cellWidth: width / 3; cellHeight: 140

            model: {
                if (!gamificationController) return []
                var all = gamificationController.achievements
                if (filterCategory === "all") return all
                var filtered = []
                for (var i = 0; i < all.length; i++) {
                    if (all[i].category === filterCategory) filtered.push(all[i])
                }
                return filtered
            }

            delegate: Item {
                width: GridView.view.cellWidth; height: GridView.view.cellHeight

                Rectangle {
                    anchors.fill: parent; anchors.margins: 6; radius: 12
                    color: modelData.unlocked ? "#FFFFFF" : "#F8FAFC"
                    border.color: modelData.unlocked ? "#BBF7D0" : "#E2E8F0"
                    border.width: modelData.unlocked ? 2 : 1
                    opacity: modelData.unlocked ? 1.0 : 0.5

                    Behavior on opacity { NumberAnimation { duration: 300 } }

                    ColumnLayout {
                        anchors.centerIn: parent; spacing: 6

                        Text {
                            text: modelData.icon; font.pixelSize: 32; Layout.alignment: Qt.AlignHCenter
                            opacity: modelData.unlocked ? 1.0 : 0.3

                            // Subtle glow for unlocked
                            layer.enabled: modelData.unlocked
                        }

                        Text {
                            text: translator ? translator.t(modelData.title_key) : modelData.title_key
                            font.pixelSize: 12; font.family: "Inter"; font.weight: Font.DemiBold
                            color: modelData.unlocked ? "#1E293B" : "#94A3B8"
                            Layout.alignment: Qt.AlignHCenter
                            Layout.maximumWidth: parent.parent.width - 16
                            elide: Text.ElideRight
                        }

                        Text {
                            text: modelData.unlocked
                                ? (translator ? translator.t(modelData.description_key) : "")
                                : "🔒"
                            font.pixelSize: 10; font.family: "Inter"
                            color: "#64748B"
                            Layout.alignment: Qt.AlignHCenter
                            Layout.maximumWidth: parent.parent.width - 16
                            elide: Text.ElideRight
                        }
                    }
                }
            }
        }
    }
}
