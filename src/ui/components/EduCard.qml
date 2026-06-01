import QtQuick
import QtQuick.Controls

Rectangle {
    id: root

    property alias content: contentArea.children
    
    // Glassmorphism default
    color: "#FFFFFF"
    opacity: 0.95
    radius: 12
    border.color: "#E2E8F0"
    border.width: 1

    Item {
        id: contentArea
        anchors.fill: parent
        anchors.margins: 16
    }
    
    // Smooth shadow effect 
    // In actual Qt6 we would use MultiEffect, but basic Rectangle is safer without QtQuick.Effects import depending on version
}
