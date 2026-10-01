import QtQuick

// A segmented control: one of options ({key, label, glyph}) is current.
Rectangle {
    id: root

    property string current
    property var options: []

    signal selected(string key)

    readonly property int padding: 3

    implicitHeight: Metrics.buttonHeight
    implicitWidth: 240 * Metrics.scale
    radius: height / 2
    color: Style.surface
    border.width: 1
    border.color: Style.line
    opacity: enabled ? 1.0 : 0.45

    Row {
        id: segments
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 2

        Repeater {
            model: root.options

            delegate: Rectangle {
                id: segment

                required property var modelData
                readonly property bool active: modelData.key === root.current

                width: root.options.length > 0
                       ? (segments.width - segments.spacing * (root.options.length - 1)) / root.options.length : 0
                height: segments.height
                radius: height / 2
                color: segment.active ? Style.accent : "transparent"
                Behavior on color { ColorAnimation { duration: Metrics.fadeDuration } }

                Rectangle {
                    anchors.fill: parent
                    radius: parent.radius
                    color: Style.press
                    opacity: tap.pressed && !segment.active ? 1 : 0
                }

                Row {
                    anchors.centerIn: parent
                    spacing: Metrics.spacingTight

                    AppIcon {
                        anchors.verticalCenter: parent.verticalCenter
                        visible: !!segment.modelData.glyph
                        name: segment.modelData.glyph || ""
                        size: Metrics.fontMedium
                        color: caption.color
                    }

                    Text {
                        id: caption
                        anchors.verticalCenter: parent.verticalCenter
                        width: Math.min(implicitWidth, segment.width - 2 * Metrics.spacingTight
                                        - (segment.modelData.glyph ? Metrics.fontMedium + Metrics.spacingTight : 0))
                        text: segment.modelData.label
                        color: segment.active ? Style.accentInk : Style.text
                        elide: Text.ElideRight
                        font.pixelSize: Metrics.fontSmall
                        font.weight: segment.active ? Font.DemiBold : Font.Normal
                    }
                }

                TapHandler {
                    id: tap
                    gesturePolicy: TapHandler.ReleaseWithinBounds
                    onTapped: if (!segment.active) root.selected(segment.modelData.key)
                }
            }
        }
    }
}
