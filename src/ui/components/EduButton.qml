import QtQuick
import QtQuick.Controls
import QtQuick.Templates as T

T.Button {
    id: control
    
    // Properties for Antigravity Premium Styling
    property color bgDefault: "#2563EB"
    property color bgHover: "#1D4ED8"
    property color bgPressed: "#1E40AF"
    property color textDefault: "#FFFFFF"
    
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    
    padding: 12
    spacing: 8

    contentItem: Text {
        text: control.text
        font.family: "Inter"
        font.pixelSize: 14
        font.weight: Font.Medium
        color: control.textDefault
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        radius: 6
        color: control.down ? control.bgPressed : (control.hovered ? control.bgHover : control.bgDefault)
        
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
        
        // Slight scale effect for modern feel (active:scale-95)
        scale: control.down ? 0.95 : (control.hovered ? 1.02 : 1.0)
        Behavior on scale {
            NumberAnimation { duration: 150; easing.type: Easing.OutQuad }
        }
    }
}
