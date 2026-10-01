import QtQuick
import QtQuick.Layouts

// A label and its value: side by side, or with stacked set, the label above a wrapping value.
Item {
    id: row

    property string glyph
    property string label
    property string value
    property bool divider: false
    property bool stacked: false

    Layout.fillWidth: true
    implicitHeight: Math.max(Metrics.rowHeight, body.implicitHeight + 2 * Metrics.spacingSmall)

    RowLayout {
        id: body
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Metrics.sideInset
        anchors.rightMargin: Metrics.sideInset
        spacing: Metrics.spacing

        AppIcon {
            Layout.preferredWidth: Metrics.iconSize
            Layout.alignment: row.stacked ? Qt.AlignTop : Qt.AlignVCenter
            visible: row.glyph !== ""
            name: row.glyph
            color: Style.accentOnSurface
        }

        GridLayout {
            Layout.fillWidth: true
            columns: row.stacked ? 1 : 2
            rowSpacing: 2
            columnSpacing: Metrics.spacing

            Text {
                Layout.fillWidth: row.stacked
                text: row.label
                color: row.stacked ? Style.textDim : Style.text
                elide: Text.ElideRight
                font.pixelSize: row.stacked ? Metrics.fontSmall : Metrics.fontNormal
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: row.stacked ? Text.AlignLeft : Text.AlignRight
                text: row.value
                color: row.stacked ? Style.text : Style.textDim
                wrapMode: row.stacked ? Text.Wrap : Text.NoWrap
                elide: row.stacked ? Text.ElideNone : Text.ElideRight
                font.pixelSize: Metrics.fontNormal
            }
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
}
