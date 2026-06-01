import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T

T.TextField {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    padding: 12
    font.family: "Inter"
    font.pixelSize: 14
    color: "#1E293B"
    selectionColor: "#2563EB"
    selectedTextColor: "#FFFFFF"
    
    // Placeholder styling
    property color placeholderColor: "#94A3B8"
    
    background: Rectangle {
        implicitWidth: 200
        implicitHeight: 40
        radius: 6
        color: "#F8FAFC"
        border.color: control.activeFocus ? "#2563EB" : "#CBD5E1"
        border.width: control.activeFocus ? 2 : 1
        
        Behavior on border.color {
            ColorAnimation { duration: 150 }
        }
    }
}
