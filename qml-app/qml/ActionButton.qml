import QtQuick

// A pill-shaped button with a label and an optional icon; primary ones are filled with the accent.
Rectangle {
    id: root

    property string glyph
    property string label
    property bool primary: false
    property color accentColor: Style.accent

    signal clicked()

    implicitHeight: Metrics.buttonHeight
    implicitWidth: content.implicitWidth + 2 * Metrics.spacingLarge * 0.75
    radius: height / 2
    color: root.primary ? root.accentColor : "transparent"
    border.width: root.primary ? 0 : 1
    border.color: Style.line
    opacity: enabled ? 1.0 : 0.45

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        color: Style.press
        opacity: tap.pressed ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Metrics.pressDuration } }
    }

    Row {
        id: content
        anchors.centerIn: parent
        spacing: Metrics.spacingSmall

        AppIcon {
            anchors.verticalCenter: parent.verticalCenter
            visible: root.glyph !== ""
            name: root.glyph
            size: Metrics.fontLarge
            color: caption.color
        }

        Text {
            id: caption
            anchors.verticalCenter: parent.verticalCenter
            width: Math.min(implicitWidth, root.width - 2 * Metrics.spacing
                            - (root.glyph !== "" ? Metrics.fontLarge + Metrics.spacingSmall : 0))
            text: root.label
            color: root.primary ? Style.accentInk : Style.accentOnSurface
            elide: Text.ElideRight
            font.pixelSize: Metrics.fontNormal
            font.weight: Font.DemiBold
        }
    }

    TapHandler {
        id: tap
        enabled: root.enabled
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: root.clicked()
    }
}
