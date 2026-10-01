import QtQuick

// A thin progress bar for value between from and to.
Rectangle {
    id: root

    property real from: 0
    property real to: 1
    property real value: 0

    readonly property real fraction: root.to > root.from
                                     ? Math.max(0, Math.min(1, (root.value - root.from) / (root.to - root.from))) : 0

    implicitWidth: 64 * Metrics.scale
    implicitHeight: Math.max(4, Math.round(6 * Metrics.scale))
    radius: height / 2
    color: Style.line

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width * root.fraction
        radius: parent.radius
        color: Style.accent
        Behavior on width { NumberAnimation { duration: Metrics.fadeDuration } }
    }
}
