import QtQuick

Item {
    id: root

    property string iconName
    property color color: Style.text
    property int iconSize: Metrics.iconSize
    readonly property bool pressed: tap.pressed

    signal clicked()

    implicitWidth: Math.max(Metrics.buttonHeight, iconSize + 2 * Metrics.spacingSmall)
    implicitHeight: implicitWidth
    opacity: enabled ? 1.0 : 0.4

    Rectangle {
        anchors.centerIn: parent
        width: Math.min(parent.width, parent.height)
        height: width
        radius: width / 2
        color: Style.press
        opacity: tap.pressed ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Metrics.pressDuration } }
    }

    AppIcon {
        anchors.centerIn: parent
        name: root.iconName
        size: root.iconSize
        color: root.color
        scale: tap.pressed ? Metrics.pressScale : 1
        Behavior on scale { NumberAnimation { duration: Metrics.pressDuration; easing.type: Easing.OutQuad } }
    }

    TapHandler {
        id: tap
        enabled: root.enabled
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: root.clicked()
    }
}
