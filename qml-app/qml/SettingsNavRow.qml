import QtQuick
import QtQuick.Layouts

// A row that opens something. Children are placed at its trailing end, before the chevron.
Item {
    id: row

    default property alias trailing: trailingRow.data
    property string glyph
    property string label
    property string value
    property bool chevron: true
    property bool divider: false
    property int rowHeight: Metrics.rowHeight

    signal tapped()

    Layout.fillWidth: true
    implicitHeight: row.rowHeight
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
        anchors.rightMargin: Metrics.sideInset - (row.chevron ? Metrics.spacingTight : 0)
        spacing: Metrics.spacing

        AppIcon {
            Layout.preferredWidth: Metrics.iconSize
            visible: row.glyph !== ""
            name: row.glyph
            color: Style.accentOnSurface
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2

            Text {
                Layout.fillWidth: true
                text: row.label
                color: Style.text
                elide: Text.ElideRight
                font.pixelSize: Metrics.fontNormal
            }

            Text {
                Layout.fillWidth: true
                visible: row.value !== ""
                text: row.value
                color: Style.textDim
                elide: Text.ElideRight
                font.pixelSize: Metrics.fontSmall
            }
        }

        Row {
            id: trailingRow
            Layout.fillHeight: true
            spacing: Metrics.spacingSmall
        }

        AppIcon {
            visible: row.chevron
            name: "chevron_right"
            color: Style.textDim
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
