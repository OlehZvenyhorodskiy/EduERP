import QtQuick

Item {
    id: loadingIndicator
    width: 40; height: 40
    
    property color indicatorColor: "#2563EB"
    property bool running: true
    
    Rectangle {
        id: spinner
        anchors.centerIn: parent
        width: parent.width; height: parent.height
        radius: width / 2
        color: "transparent"
        border.color: "#E2E8F0"
        border.width: 3
        
        Rectangle {
            width: parent.width; height: parent.height
            radius: width / 2
            color: "transparent"
            border.color: indicatorColor
            border.width: 3
            
            // Only show a quarter arc — clip trick
            clip: true
            Rectangle {
                width: parent.width / 2; height: parent.height
                color: parent.parent.parent.parent.color || "#FFFFFF"
                anchors.right: parent.right
            }
            Rectangle {
                width: parent.width; height: parent.height / 2
                color: parent.parent.parent.parent.color || "#FFFFFF"
                anchors.bottom: parent.bottom
            }
        }
        
        RotationAnimation on rotation {
            from: 0; to: 360
            duration: 1000
            loops: Animation.Infinite
            running: loadingIndicator.running
        }
    }
}
