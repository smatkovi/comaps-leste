import QtQuick
import QtQuick.Layouts

// An action row in a card: icon and label in the accent colour.
Item {
    id: row

    property string glyph
    property string label
    property bool divider: false

    signal tapped()

    Layout.fillWidth: true
    implicitHeight: Metrics.rowHeight
    opacity: enabled ? 1.0 : 0.45

    Rectangle {
        anchors.fill: parent
        color: Style.press
        opacity: tap.pressed ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Metrics.pressDuration } }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Metrics.sideInset
        anchors.rightMargin: Metrics.sideInset
        spacing: Metrics.spacing

        AppIcon {
            Layout.preferredWidth: Metrics.iconSize
            visible: row.glyph !== ""
            name: row.glyph
            color: Style.accentOnSurface
        }

        Text {
            Layout.fillWidth: true
            text: row.label
            color: Style.accentOnSurface
            elide: Text.ElideRight
            font.pixelSize: Metrics.fontNormal
            font.weight: Font.Medium
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Metrics.sideInset + (row.glyph !== "" ? Metrics.iconSize + Metrics.spacing : 0)
        height: 1
        visible: row.divider
        color: Style.line
    }

    TapHandler {
        id: tap
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: row.tapped()
    }
}
