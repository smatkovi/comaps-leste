import QtQuick

// An icon button: filled with baseColor while lit, a plain bordered surface otherwise.
Item {
    id: root

    property string glyph
    property bool round: true
    property int diameter: Metrics.buttonHeight
    property real glyphScale: 0.5
    property bool lit: true
    property color baseColor: Style.accent
    property color litInk: Style.accentInk
    property color plainColor: Style.surfaceRaised
    property color plainBorder: Style.line

    signal tapped()

    implicitWidth: root.diameter
    implicitHeight: root.diameter
    opacity: enabled ? 1.0 : 0.45

    Rectangle {
        id: background
        anchors.fill: parent
        radius: root.round ? width / 2 : Metrics.radius
        color: root.lit ? root.baseColor : root.plainColor
        border.width: root.lit ? 0 : 1
        border.color: root.plainBorder
        scale: tap.pressed ? Metrics.pressScale : 1
        Behavior on color { ColorAnimation { duration: Metrics.fadeDuration } }
        Behavior on scale { NumberAnimation { duration: Metrics.pressDuration; easing.type: Easing.OutQuad } }

        Rectangle {
            anchors.fill: parent
            radius: parent.radius
            color: Style.press
            opacity: tap.pressed ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Metrics.pressDuration } }
        }

        AppIcon {
            anchors.centerIn: parent
            name: root.glyph
            size: Math.round(root.diameter * root.glyphScale)
            color: root.lit ? root.litInk : Style.text
        }
    }

    TapHandler {
        id: tap
        enabled: root.enabled
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: root.tapped()
    }
}
