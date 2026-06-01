import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: confirmDialog
    
    property string title: "Bevestiging"
    property string message: ""
    property string confirmText: "Bevestigen"
    property string cancelText: "Annuleren"
    property string variant: "default" // default, danger
    
    signal confirmed()
    signal cancelled()
    
    anchors.centerIn: parent
    width: 400
    height: dialogContent.implicitHeight + 80
    modal: true
    dim: true
    
    background: Rectangle {
        radius: 12
        color: "#FFFFFF"
        border.color: "#E2E8F0"
    }
    
    Overlay.modal: Rectangle {
        color: "#00000060"
    }
    
    ColumnLayout {
        id: dialogContent
        anchors.fill: parent; anchors.margins: 24; spacing: 16
        
        Text {
            text: title
            font.pixelSize: 18; font.family: "Inter"; font.weight: Font.Bold
            color: "#1E293B"
        }
        
        Text {
            text: message
            font.pixelSize: 14; font.family: "Inter"
            color: "#475569"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        
        Item { Layout.preferredHeight: 8 }
        
        RowLayout {
            Layout.fillWidth: true; spacing: 12
            
            Item { Layout.fillWidth: true }
            
            Rectangle {
                width: cancelBtnText.implicitWidth + 32; height: 36; radius: 8
                color: "#F1F5F9"; border.color: "#E2E8F0"
                Text { id: cancelBtnText; anchors.centerIn: parent; text: cancelText; font.pixelSize: 14; font.family: "Inter"; color: "#475569" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { cancelled(); confirmDialog.close() } }
            }
            
            Rectangle {
                width: confirmBtnText.implicitWidth + 32; height: 36; radius: 8
                color: variant === "danger" ? "#DC2626" : "#2563EB"
                Text { id: confirmBtnText; anchors.centerIn: parent; text: confirmText; font.pixelSize: 14; font.family: "Inter"; font.weight: Font.Medium; color: "#FFFFFF" }
                MouseArea { anchors.fill: parent; cursorShape: Qt.PointingHandCursor; onClicked: { confirmed(); confirmDialog.close() } }
            }
        }
    }
}
