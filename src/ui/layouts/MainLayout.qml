import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    
    property alias currentView: contentLoader.source
    property bool sidebarCollapsed: false
    
    color: Theme.bgSurface
    
    RowLayout {
        anchors.fill: parent
        spacing: 0
        
        // Sidebar
        Sidebar {
            id: sidebar
            Layout.fillHeight: true
            Layout.preferredWidth: sidebarCollapsed ? 64 : 240
            
            Behavior on Layout.preferredWidth {
                NumberAnimation { duration: 200; easing.type: Easing.OutQuad }
            }
        }
        
        // Main Content Area
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0
            
            // Top Bar
            TopBar {
                Layout.fillWidth: true
                Layout.preferredHeight: 56
                
                onToggleSidebar: {
                    sidebarCollapsed = !sidebarCollapsed
                }
            }
            
            // Content Area
            ContentArea {
                Layout.fillWidth: true
                Layout.fillHeight: true
                
                Loader {
                    id: contentLoader
                    anchors.fill: parent
                    anchors.margins: 24
                }
            }
        }
    }
}
