import QtQuick
import QtQuick.Layouts

Rectangle {
    id: emptyState
    
    property string icon: "📭"
    property string title: "Geen gegevens"
    property string subtitle: ""
    property string actionLabel: ""
    
    signal actionClicked()
    
    color: "transparent"
    
    ColumnLayout {
        anchors.centerIn: parent
        spacing: 12
        
        Text {
            text: icon
            font.pixelSize: 48
            Layout.alignment: Qt.AlignHCenter
        }
        
        Text {
            text: title
            font.pixelSize: 16
            font.family: "Inter"
            font.weight: Font.Medium
            color: "#64748B"
            Layout.alignment: Qt.AlignHCenter
        }
        
        Text {
            text: subtitle
            font.pixelSize: 13
            font.family: "Inter"
            color: "#94A3B8"
            Layout.alignment: Qt.AlignHCenter
            visible: subtitle !== ""
        }
        
        // Optional action button
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            visible: actionLabel !== ""
            width: actionText.implicitWidth + 32
            height: 36; radius: 8
            color: "#2563EB"
            
            scale: actionMouse.pressed ? 0.95 : (actionMouse.containsMouse ? 1.02 : 1.0)
            Behavior on scale { NumberAnimation { duration: 150 } }
            
            Text {
                id: actionText
                anchors.centerIn: parent
                text: actionLabel
                font.pixelSize: 13; font.family: "Inter"; font.weight: Font.Medium
                color: "#FFFFFF"
            }
            
            MouseArea {
                id: actionMouse
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: emptyState.actionClicked()
            }
        }
    }
}
