pragma Singleton

import QtQuick

QtObject {
    readonly property real scale: Style.scaleFactor

    readonly property int spacingTight: 4 * scale
    readonly property int spacingSmall: 8 * scale
    readonly property int spacing: 12 * scale
    readonly property int spacingLarge: 24 * scale
    readonly property int sideInset: 14 * scale
    readonly property int pageMargin: 16 * scale
    readonly property int headerGap: 18 * scale

    readonly property int radius: 7 * scale
    readonly property int radiusLarge: 12 * scale
    readonly property int radiusSheet: 18 * scale
    readonly property int radiusWidget: 24 * scale

    readonly property int rowHeight: 58 * scale
    readonly property int buttonHeight: 44 * scale
    readonly property int mapButtonSize: 54 * scale
    readonly property int panelHeight: 64 * scale
    readonly property int iconSize: fontLarge

    readonly property int fontTiny: Style.fontTiny * scale
    readonly property int fontSmall: Style.fontSmall * scale
    readonly property int fontNormal: Style.fontNormal * scale
    readonly property int fontMedium: Style.fontMedium * scale
    readonly property int fontLarge: Style.fontLarge * scale
    readonly property int fontHuge: Style.fontHuge * scale

    readonly property color background: Style.dark ? "black" : "#eef1f5"

    readonly property real pressScale: 0.94
    readonly property int pressDuration: 110
    readonly property int fadeDuration: 160

    readonly property bool needsBackButton: true
}
