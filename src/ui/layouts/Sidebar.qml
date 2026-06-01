import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: sidebar
    
    color: "#0F172A"  // Dark sidebar (slate-900)
    
    property int currentIndex: 0
    
    signal navigationRequested(string viewPath)
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        spacing: 4
        
        // App Logo
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            color: "transparent"
            
            Text {
                anchors.centerIn: parent
                text: sidebarCollapsed ? "E" : "EduERP"
                font.pixelSize: sidebarCollapsed ? 20 : 18
                font.family: "Inter"
                font.weight: Font.Bold
                color: "#F8FAFC"
            }
        }
        
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: "#1E293B"
        }
        
        // Navigation Items
        Repeater {
            model: ListModel {
                ListElement { label: "Dashboard"; icon: "🏠"; viewPath: "views/dashboard/DashboardView.qml" }
                ListElement { label: "Bedrijf"; icon: "🏢"; viewPath: "views/company/CompanyListView.qml" }
                ListElement { label: "Simulatie"; icon: "⚙️"; viewPath: "views/simulation/SimulationView.qml" }
                ListElement { label: "Berichten"; icon: "💬"; viewPath: "views/messaging/ConversationsView.qml" }
                ListElement { label: "Vrienden"; icon: "👥"; viewPath: "views/social/FriendsView.qml" }
                ListElement { label: "Profiel"; icon: "👤"; viewPath: "views/profile/ProfileView.qml" }
            }
            
            delegate: Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 44
                radius: 8
                color: currentIndex === index ? "#1E40AF" : (mouseArea.containsMouse ? "#1E293B" : "transparent")
                
                Behavior on color {
                    ColorAnimation { duration: 150 }
                }
                
                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 12
                    
                    Text {
                        text: model.icon
                        font.pixelSize: 18
                    }
                    
                    Text {
                        text: model.label
                        font.pixelSize: 14
                        font.family: "Inter"
                        font.weight: currentIndex === index ? Font.DemiBold : Font.Normal
                        color: currentIndex === index ? "#FFFFFF" : "#94A3B8"
                        visible: !sidebarCollapsed
                        
                        Layout.fillWidth: true
                    }
                }
                
                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        currentIndex = index
                        navigationRequested(model.viewPath)
                    }
                }
            }
        }
        
        Item { Layout.fillHeight: true }
        
        // Settings button at bottom
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: 8
            color: settingsMouseArea.containsMouse ? "#1E293B" : "transparent"
            
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                spacing: 12
                
                Text {
                    text: "⚙️"
                    font.pixelSize: 18
                }
                
                Text {
                    text: "Instellingen"
                    font.pixelSize: 14
                    font.family: "Inter"
                    color: "#94A3B8"
                    visible: !sidebarCollapsed
                }
            }
            
            MouseArea {
                id: settingsMouseArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: navigationRequested("views/profile/SettingsView.qml")
            }
        }
    }
}
