import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    minimumWidth: 1024
    minimumHeight: 720
    visible: true
    title: translator ? ("EduERP — " + translator.t("app.tagline")) : "EduERP"
    color: "#F8FAFC"

    property bool isLoggedIn: false
    property string currentViewPath: "views/dashboard/DashboardView.qml"
    property bool sidebarCollapsed: false
    // Reactive locale binding — changing this triggers all t() re-evaluations
    property string appLocale: translator ? translator.currentLocale : "nl-BE"

    // Auth check — show login or main interface
    Loader {
        anchors.fill: parent
        source: isLoggedIn ? "" : "views/auth/LoginView.qml"
        active: !isLoggedIn
    }

    // Main Application Shell (visible after login)
    RowLayout {
        anchors.fill: parent
        spacing: 0
        visible: isLoggedIn

        // Sidebar
        Rectangle {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: sidebarCollapsed ? 64 : 240
            color: "#0F172A"

            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 4

                // Logo
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 48; color: "transparent"
                    Text {
                        anchors.centerIn: parent
                        text: sidebarCollapsed ? "E" : "EduERP"
                        font.pixelSize: sidebarCollapsed ? 20 : 18
                        font.family: "Inter"; font.weight: Font.Bold; color: "#F8FAFC"
                    }
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#1E293B" }

                // Nav items — labels come from translator for i18n
                property int currentNav: 0
                Repeater {
                    model: ListModel {
                        ListElement { tKey: "nav.dashboard"; icon: "🏠"; path: "views/dashboard/DashboardView.qml" }
                        ListElement { tKey: "nav.company"; icon: "🏢"; path: "views/company/CompanyListView.qml" }
                        ListElement { tKey: "nav.simulation"; icon: "⚙️"; path: "views/simulation/SimulationView.qml" }
                        ListElement { tKey: "nav.messages"; icon: "💬"; path: "views/messaging/ConversationsView.qml" }
                        ListElement { tKey: "nav.friends"; icon: "👥"; path: "views/social/FriendsView.qml" }
                        ListElement { tKey: "nav.achievements"; icon: "🏅"; path: "views/gamification/AchievementsView.qml" }
                        ListElement { tKey: "nav.profile"; icon: "👤"; path: "views/profile/ProfileView.qml" }
                        ListElement { tKey: "nav.leaderboard"; icon: "🏆"; path: "views/leaderboard/LeaderboardView.qml" }
                    }

                    delegate: Rectangle {
                        Layout.fillWidth: true; Layout.preferredHeight: 44
                        radius: 8
                        color: parent.parent.currentNav === index ? "#1E40AF" : (navMouse.containsMouse ? "#1E293B" : "transparent")
                        Behavior on color { ColorAnimation { duration: 150 } }

                        RowLayout {
                            anchors.fill: parent; anchors.leftMargin: 12; anchors.rightMargin: 12; spacing: 12
                            Text { text: model.icon; font.pixelSize: 18 }
                            Text {
                                // Reactive: re-evaluates when locale changes
                                text: translator ? translator.t(model.tKey) : model.tKey
                                font.pixelSize: 14; font.family: "Inter"
                                font.weight: parent.parent.parent.parent.currentNav === index ? Font.DemiBold : Font.Normal
                                color: parent.parent.parent.parent.currentNav === index ? "#FFFFFF" : "#94A3B8"
                                visible: !sidebarCollapsed
                                Layout.fillWidth: true
                            }
                        }

                        MouseArea {
                            id: navMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                sidebar.children[0].currentNav = index
                                currentViewPath = model.path
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                // Settings
                Rectangle {
                    Layout.fillWidth: true; Layout.preferredHeight: 44; radius: 8
                    color: settingsMouse.containsMouse ? "#1E293B" : "transparent"
                    RowLayout {
                        anchors.fill: parent; anchors.leftMargin: 12; spacing: 12
                        Text { text: "⚙️"; font.pixelSize: 18 }
                        Text { text: translator ? translator.t("nav.settings") : "Settings"; font.pixelSize: 14; font.family: "Inter"; color: "#94A3B8"; visible: !sidebarCollapsed }
                    }
                    MouseArea { id: settingsMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: currentViewPath = "views/profile/SettingsView.qml" }
                }
            }
        }

        // Main content area
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0

            // Top Bar
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 56; color: "#FFFFFF"
                Rectangle { anchors.bottom: parent.bottom; width: parent.width; height: 1; color: "#E2E8F0" }

                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 16

                    Rectangle {
                        width: 36; height: 36; radius: 8; color: hambMouse.containsMouse ? "#F1F5F9" : "transparent"
                        Text { anchors.centerIn: parent; text: "☰"; font.pixelSize: 18; color: "#475569" }
                        MouseArea { id: hambMouse; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor; onClicked: sidebarCollapsed = !sidebarCollapsed }
                    }

                    Rectangle {
                        Layout.fillWidth: true; Layout.maximumWidth: 400; height: 36; radius: 8; color: "#F1F5F9"
                        Text { anchors.centerIn: parent; text: "🔍 " + (translator ? translator.t("common.search") + "..." : "Search..."); font.pixelSize: 13; font.family: "Inter"; color: "#94A3B8" }
                    }

                    Item { Layout.fillWidth: true }

                    Rectangle {
                        width: 36; height: 36; radius: 18; color: bellMouse2.containsMouse ? "#F1F5F9" : "transparent"
                        Text { anchors.centerIn: parent; text: "🔔"; font.pixelSize: 18 }
                        Rectangle { anchors.right: parent.right; anchors.top: parent.top; width: 8; height: 8; radius: 4; color: "#EF4444" }
                        MouseArea { id: bellMouse2; anchors.fill: parent; hoverEnabled: true; cursorShape: Qt.PointingHandCursor }
                    }

                    Rectangle {
                        width: 36; height: 36; radius: 18; color: "#2563EB"
                        Text { anchors.centerIn: parent; text: "J"; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.Bold; color: "#FFFFFF" }
                        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor }
                    }
                }
            }

            // Content
            Rectangle {
                Layout.fillWidth: true; Layout.fillHeight: true; color: "#F8FAFC"
                Loader {
                    anchors.fill: parent; anchors.margins: 24
                    source: currentViewPath
                }
            }
        }
    }

    // DEV: Auto-login for development
    Component.onCompleted: {
        isLoggedIn = true
    }
}
