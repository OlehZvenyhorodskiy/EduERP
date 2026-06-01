import QtQuick

Rectangle {
    id: badge
    
    property string text: ""
    property string variant: "primary" // primary, success, warning, error, neutral
    
    width: badgeText.implicitWidth + 16
    height: 22
    radius: 11
    
    color: {
        switch (variant) {
            case "primary": return "#EFF6FF"
            case "success": return "#F0FDF4"
            case "warning": return "#FEF3C7"
            case "error": return "#FEF2F2"
            case "neutral": return "#F1F5F9"
            default: return "#F1F5F9"
        }
    }
    
    Text {
        id: badgeText
        anchors.centerIn: parent
        text: badge.text
        font.pixelSize: 11
        font.family: "Inter"
        font.weight: Font.Medium
        color: {
            switch (variant) {
                case "primary": return "#2563EB"
                case "success": return "#16A34A"
                case "warning": return "#D97706"
                case "error": return "#DC2626"
                case "neutral": return "#475569"
                default: return "#475569"
            }
        }
    }
}
