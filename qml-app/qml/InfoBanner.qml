import QtQuick
import QtQuick.Layouts

// A tinted note; kind is "accent", "warn" or "danger".
Rectangle {
    id: root

    property string kind: "accent"
    property string glyph
    property string text

    readonly property color tone: root.kind === "danger" ? Style.danger
                                : root.kind === "warn" ? Style.warn
                                : Style.accentOnSurface

    Layout.fillWidth: true
    implicitHeight: row.implicitHeight + 2 * Metrics.spacing
    radius: Metrics.radius
    color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.12)
    border.width: 1
    border.color: Qt.rgba(root.tone.r, root.tone.g, root.tone.b, 0.35)

    RowLayout {
        id: row
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Metrics.spacing
        anchors.rightMargin: Metrics.spacing
        spacing: Metrics.spacingSmall

        AppIcon {
            Layout.alignment: Qt.AlignTop
            visible: root.glyph !== ""
            name: root.glyph
            size: Metrics.fontLarge
            color: root.tone
        }

        Text {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            text: root.text
            color: Style.text
            wrapMode: Text.Wrap
            font.pixelSize: Metrics.fontSmall
        }
    }
}
