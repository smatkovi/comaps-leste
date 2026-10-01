import QtQuick

AccentFill {
    id: root

    property string glyph

    implicitWidth: Metrics.buttonHeight
    implicitHeight: Metrics.buttonHeight
    radius: Metrics.radius

    AppIcon {
        anchors.centerIn: parent
        name: root.glyph
        size: Math.round(root.height * 0.55)
        color: Style.accentInk
    }
}
