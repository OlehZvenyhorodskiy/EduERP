import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: loginView
    color: "#0F172A"
    
    // Center card
    Rectangle {
        anchors.centerIn: parent
        width: 420
        height: 500
        radius: 16
        color: "#FFFFFF"
        
        // Glass edge
        border.color: "#E2E8F0"
        border.width: 1
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 40
            spacing: 24
            
            // Logo
            Text {
                text: "EduERP"
                font.pixelSize: 32
                font.family: "Inter"
                font.weight: Font.Bold
                color: "#2563EB"
                Layout.alignment: Qt.AlignHCenter
            }
            
            Text {
                text: translator ? translator.t("app.tagline") : "Simulatie voor het onderwijs"
                font.pixelSize: 14
                font.family: "Inter"
                color: "#64748B"
                Layout.alignment: Qt.AlignHCenter
            }
            
            Item { Layout.preferredHeight: 16 }
            
            // Google login button
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                radius: 8
                color: googleMouse.containsMouse ? "#F1F5F9" : "#FFFFFF"
                border.color: "#E2E8F0"
                border.width: 1
                
                Behavior on color { ColorAnimation { duration: 150 } }
                
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    
                    Text { text: "🔵"; font.pixelSize: 20 }
                    
                    Text {
                        text: translator ? translator.t("auth.login_google") : "Sign in with Google"
                        font.pixelSize: 14
                        font.family: "Inter"
                        font.weight: Font.Medium
                        color: "#1E293B"
                    }
                }
                
                MouseArea {
                    id: googleMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // Trigger OAuth flow via AuthController
                        console.log("Google OAuth login initiated")
                    }
                }
            }
            
            // Microsoft login button
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                radius: 8
                color: msMouse.containsMouse ? "#F1F5F9" : "#FFFFFF"
                border.color: "#E2E8F0"
                border.width: 1
                
                Behavior on color { ColorAnimation { duration: 150 } }
                
                RowLayout {
                    anchors.centerIn: parent
                    spacing: 12
                    
                    Text { text: "🟠"; font.pixelSize: 20 }
                    
                    Text {
                        text: translator ? translator.t("auth.login_microsoft") : "Sign in with Microsoft"
                        font.pixelSize: 14
                        font.family: "Inter"
                        font.weight: Font.Medium
                        color: "#1E293B"
                    }
                }
                
                MouseArea {
                    id: msMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        console.log("Microsoft OAuth login initiated")
                    }
                }
            }
            
            Item { Layout.fillHeight: true }
            
            // Footer
            Text {
                text: translator ? translator.t("auth.login_subtitle") : "Use your school account to sign in"
                font.pixelSize: 12
                font.family: "Inter"
                color: "#94A3B8"
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
}
