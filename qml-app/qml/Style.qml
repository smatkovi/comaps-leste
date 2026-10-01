pragma Singleton

import QtQuick

// Colours and base font sizes. Dark unless the theme setting is light, or follows the desktop's
// colour scheme with the theme set to system.
QtObject {
    readonly property bool dark: Settings.theme === "system" ? Qt.styleHints.colorScheme === Qt.Dark
                                                             : Settings.theme !== "light"
    readonly property real scaleFactor: 1.0

    readonly property color accent: dark ? "#4f8ff7" : "#1a73e8"
    readonly property color accentInk: "#ffffff"
    readonly property color accentOnSurface: dark ? "#8ab4f8" : "#1a73e8"

    readonly property color text: dark ? "#f1f3f4" : "#15181d"
    readonly property color textMuted: dark ? "#bdc1c6" : "#454b54"
    readonly property color textDim: dark ? "#9aa0a6" : "#5f6670"

    readonly property color ok: dark ? "#81c995" : "#188038"
    readonly property color warn: dark ? "#fdd663" : "#e37400"
    readonly property color danger: dark ? "#f28b82" : "#d93025"

    readonly property color line: dark ? "#2f333a" : "#d7dce3"
    readonly property color press: dark ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(0, 0, 0, 0.08)
    readonly property color surface: dark ? "#16181c" : "#ffffff"
    readonly property color surfaceRaised: dark ? "#1f2228" : "#ffffff"

    readonly property int fontTiny: 11
    readonly property int fontSmall: 13
    readonly property int fontNormal: 15
    readonly property int fontMedium: 17
    readonly property int fontLarge: 20
    readonly property int fontHuge: 28
}
