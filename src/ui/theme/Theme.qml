pragma Singleton
import QtQuick

// Theme.qml — Singleton providing design tokens across the entire application.
// Switch between LightTheme and DarkTheme via ThemeManager.

QtObject {
    // ── Surface Colors ──
    readonly property color bgSurface: "#F8FAFC"
    readonly property color bgCard: "#FFFFFF"
    readonly property color bgSidebar: "#0F172A"
    readonly property color bgInput: "#F1F5F9"
    readonly property color bgOverlay: "#00000080"

    // ── Brand Colors ──
    readonly property color colorPrimary: "#2563EB"
    readonly property color colorPrimaryHover: "#1D4ED8"
    readonly property color colorPrimaryPressed: "#1E40AF"
    readonly property color colorSecondary: "#7C3AED"
    readonly property color colorSuccess: "#16A34A"
    readonly property color colorWarning: "#D97706"
    readonly property color colorError: "#DC2626"

    // ── Text Colors ──
    readonly property color textPrimary: "#1E293B"
    readonly property color textSecondary: "#475569"
    readonly property color textMuted: "#94A3B8"
    readonly property color textOnPrimary: "#FFFFFF"

    // ── Border Colors ──
    readonly property color borderDefault: "#E2E8F0"
    readonly property color borderFocus: "#2563EB"

    // ── Typography ──
    readonly property string fontFamily: "Inter"
    readonly property int fontSizeXs: 12
    readonly property int fontSizeSm: 13
    readonly property int fontSizeMd: 14
    readonly property int fontSizeLg: 16
    readonly property int fontSizeXl: 20
    readonly property int fontSize2xl: 24
    readonly property int fontSize3xl: 30

    // ── Spacing ──
    readonly property int spacingXs: 4
    readonly property int spacingSm: 8
    readonly property int spacingMd: 12
    readonly property int spacingLg: 16
    readonly property int spacingXl: 24
    readonly property int spacing2xl: 32

    // ── Radii ──
    readonly property int radiusSm: 4
    readonly property int radiusMd: 6
    readonly property int radiusLg: 8
    readonly property int radiusXl: 12
    readonly property int radiusFull: 999

    // ── Shadows (approximated as Rectangle border/color) ──
    readonly property color shadowSm: "#0000000D"
    readonly property color shadowMd: "#00000014"

    // ── Animation Durations ──
    readonly property int durationFast: 100
    readonly property int durationNormal: 200
    readonly property int durationSlow: 300
}
