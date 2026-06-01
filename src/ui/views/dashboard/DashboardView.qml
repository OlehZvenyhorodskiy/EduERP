import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: dashboardView
    color: "transparent"
    
    ScrollView {
        anchors.fill: parent
        clip: true
        
        ColumnLayout {
            width: dashboardView.width
            spacing: 24
            
            // Header
            RowLayout {
                Layout.fillWidth: true
                
                ColumnLayout {
                    spacing: 4
                    
                    Text {
                        text: translator ? translator.tp("dashboard.welcome", ["Jan"]) : "Welcome!"
                        font.pixelSize: 24
                        font.family: "Inter"
                        font.weight: Font.Bold
                        color: "#1E293B"
                    }
                    
                    Text {
                        text: translator ? translator.t("dashboard.overview") : "Overview"
                        font.pixelSize: 14
                        font.family: "Inter"
                        color: "#64748B"
                    }
                }
                
                Item { Layout.fillWidth: true }
            }
            
            // KPI Cards Row
            RowLayout {
                Layout.fillWidth: true
                spacing: 16
                
                // Revenue Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 12
                    color: "#FFFFFF"
                    border.color: "#E2E8F0"
                    border.width: 1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 8
                        
                        Text {
                            text: "💰 " + (translator ? translator.t("dashboard.revenue") : "Revenue")
                            font.pixelSize: 13
                            font.family: "Inter"
                            color: "#64748B"
                        }
                        
                        Text {
                            text: "€ 0,00"
                            font.pixelSize: 28
                            font.family: "Inter"
                            font.weight: Font.Bold
                            color: "#1E293B"
                        }
                        
                        Text {
                            text: "↗ +0% " + (translator ? translator.t("dashboard.this_month") : "this month")
                            font.pixelSize: 12
                            font.family: "Inter"
                            color: "#16A34A"
                        }
                    }
                }
                
                // Profit Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 12
                    color: "#FFFFFF"
                    border.color: "#E2E8F0"
                    border.width: 1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 8
                        
                        Text {
                            text: "📊 " + (translator ? translator.t("dashboard.profit") : "Profit")
                            font.pixelSize: 13
                            font.family: "Inter"
                            color: "#64748B"
                        }
                        
                        Text {
                            text: "€ 0,00"
                            font.pixelSize: 28
                            font.family: "Inter"
                            font.weight: Font.Bold
                            color: "#1E293B"
                        }
                        
                        Text {
                            text: "→ " + (translator ? translator.t("dashboard.stable") : "Stable")
                            font.pixelSize: 12
                            font.family: "Inter"
                            color: "#D97706"
                        }
                    }
                }
                
                // Cash Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 12
                    color: "#FFFFFF"
                    border.color: "#E2E8F0"
                    border.width: 1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 8
                        
                        Text {
                            text: "🏦 " + (translator ? translator.t("dashboard.cash") : "Cash")
                            font.pixelSize: 13
                            font.family: "Inter"
                            color: "#64748B"
                        }
                        
                        Text {
                            text: "€ 100.000,00"
                            font.pixelSize: 28
                            font.family: "Inter"
                            font.weight: Font.Bold
                            color: "#1E293B"
                        }
                        
                        Text {
                            text: translator ? translator.t("dashboard.starting_capital") : "Starting capital"
                            font.pixelSize: 12
                            font.family: "Inter"
                            color: "#64748B"
                        }
                    }
                }
                
                // Satisfaction Card
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 120
                    radius: 12
                    color: "#FFFFFF"
                    border.color: "#E2E8F0"
                    border.width: 1
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 8
                        
                        Text {
                            text: "😊 " + (translator ? translator.t("dashboard.satisfaction") : "Satisfaction")
                            font.pixelSize: 13
                            font.family: "Inter"
                            color: "#64748B"
                        }
                        
                        Text {
                            text: "50%"
                            font.pixelSize: 28
                            font.family: "Inter"
                            font.weight: Font.Bold
                            color: "#1E293B"
                        }
                        
                        Text {
                            text: "Gemiddeld"
                            font.pixelSize: 12
                            font.family: "Inter"
                            color: "#D97706"
                        }
                    }
                }
            }
            
            // Activity / Recent Events Section
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 300
                radius: 12
                color: "#FFFFFF"
                border.color: "#E2E8F0"
                border.width: 1
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16
                    
                    Text {
                        text: "📋 " + (translator ? translator.t("dashboard.recent_activity") : "Recent Activity")
                        font.pixelSize: 16
                        font.family: "Inter"
                        font.weight: Font.DemiBold
                        color: "#1E293B"
                    }
                    
                    // Empty state
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#F8FAFC"
                        radius: 8
                        
                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 8
                            
                            Text {
                                text: "🚀"
                                font.pixelSize: 40
                                Layout.alignment: Qt.AlignHCenter
                            }
                            
                            Text {
                                text: translator ? translator.t("dashboard.no_activity") : "No activity yet"
                                font.pixelSize: 14
                                font.family: "Inter"
                                font.weight: Font.Medium
                                color: "#64748B"
                                Layout.alignment: Qt.AlignHCenter
                            }
                            
                            Text {
                                text: translator ? translator.t("dashboard.start_hint") : "Start by creating your first company"
                                font.pixelSize: 13
                                font.family: "Inter"
                                color: "#94A3B8"
                                Layout.alignment: Qt.AlignHCenter
                            }
                        }
                    }
                }
            }
        }
    }
}
