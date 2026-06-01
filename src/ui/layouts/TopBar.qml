import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: topBar
    
    color: "#FFFFFF"
    border.color: "#E2E8F0"
    border.width: 0
    
    signal toggleSidebar()
    
    // Bottom border shadow
    Rectangle {
        anchors.bottom: parent.bottom
        width: parent.width
        height: 1
        color: "#E2E8F0"
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        spacing: 16
        
        // Hamburger menu
        Rectangle {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            radius: 8
            color: hamburgerMouse.containsMouse ? "#F1F5F9" : "transparent"
            
            Text {
                anchors.centerIn: parent
                text: "☰"
                font.pixelSize: 18
                color: "#475569"
            }
            
            MouseArea {
                id: hamburgerMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: toggleSidebar()
            }
        }
        
        // Search bar
        Rectangle {
            Layout.fillWidth: true
            Layout.maximumWidth: 400
            Layout.preferredHeight: 36
            radius: 8
            color: "#F1F5F9"
            border.color: searchField.activeFocus ? "#2563EB" : "transparent"
            border.width: searchField.activeFocus ? 2 : 0
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                
                Text {
                    text: "🔍"
                    font.pixelSize: 14
                    color: "#94A3B8"
                }
                
                TextInput {
                    id: searchField
                    Layout.fillWidth: true
                    font.pixelSize: 14
                    font.family: "Inter"
                    color: "#1E293B"
                    clip: true
                    
                    Text {
                        anchors.fill: parent
                        text: "Zoeken..."
                        font.pixelSize: 14
                        font.family: "Inter"
                        color: "#94A3B8"
                        visible: !searchField.text && !searchField.activeFocus
                    }
                }
            }
        }
        
        Item { Layout.fillWidth: true }
        
        // Notification bell
        Rectangle {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            radius: 18
            color: bellMouse.containsMouse ? "#F1F5F9" : "transparent"
            
            Text {
                anchors.centerIn: parent
                text: "🔔"
                font.pixelSize: 18
            }
            
            // Notification badge
            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                width: 8
                height: 8
                radius: 4
                color: "#EF4444"
                visible: true // Show when there are notifications
            }
            
            MouseArea {
                id: bellMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
            }
        }
        
        // User avatar
        Rectangle {
            Layout.preferredWidth: 36
            Layout.preferredHeight: 36
            radius: 18
            color: "#2563EB"
            
            Text {
                anchors.centerIn: parent
                text: "J"
                font.pixelSize: 14
                font.family: "Inter"
                font.weight: Font.Bold
                color: "#FFFFFF"
            }
            
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
}
