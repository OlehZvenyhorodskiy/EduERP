import QtQuick
import QtQuick.Layouts

Rectangle {
    id: notification
    
    property string title: ""
    property string message: ""
    property string variant: "info" // info, success, warning, error
    property bool visible: false
    
    width: 360
    height: notifContent.implicitHeight + 32
    radius: 10
    opacity: visible ? 1.0 : 0.0
    
    Behavior on opacity { NumberAnimation { duration: 250; easing.type: Easing.OutQuad } }
    
    color: {
        switch (variant) {
            case "info": return "#EFF6FF"
            case "success": return "#F0FDF4"
            case "warning": return "#FFFBEB"
            case "error": return "#FEF2F2"
            default: return "#F8FAFC"
        }
    }
    
    border.color: {
        switch (variant) {
            case "info": return "#BFDBFE"
            case "success": return "#BBF7D0"
            case "warning": return "#FDE68A"
            case "error": return "#FECACA"
            default: return "#E2E8F0"
        }
    }
    
    ColumnLayout {
        id: notifContent
        anchors.fill: parent; anchors.margins: 16; spacing: 4
        
        Text {
            text: {
                var icon = ""
                switch (variant) {
                    case "info": icon = "ℹ️"; break
                    case "success": icon = "✅"; break
                    case "warning": icon = "⚠️"; break
                    case "error": icon = "❌"; break
                }
                return icon + " " + title
            }
            font.pixelSize: 14; font.family: "Inter"; font.weight: Font.DemiBold
            color: "#1E293B"
        }
        
        Text {
            text: message
            font.pixelSize: 13; font.family: "Inter"
            color: "#475569"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
    }
    
    // Close button
    Text {
        anchors.right: parent.right; anchors.top: parent.top
        anchors.margins: 10
        text: "✕"; font.pixelSize: 14; color: "#94A3B8"
        MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: notification.visible = false }
    }
    
    // Auto-hide after 5 seconds
    Timer {
        running: notification.visible; interval: 5000; onTriggered: notification.visible = false
    }
}
