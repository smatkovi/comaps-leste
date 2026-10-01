import QtQuick
import QtQuick.Layouts

Item {
    id: row

    property string glyph
    property string label
    property string description
    property bool checked: false
    property bool divider: false

    signal toggled(bool on)

    Layout.fillWidth: true
    implicitHeight: Math.max(Metrics.rowHeight, texts.implicitHeight + 2 * Metrics.spacingSmall)
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

        ColumnLayout {
            id: texts
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
                visible: row.description !== ""
                text: row.description
                color: Style.textDim
                wrapMode: Text.Wrap
                font.pixelSize: Metrics.fontSmall
            }
        }

        Rectangle {
            id: track
            implicitWidth: Math.round(44 * Metrics.scale)
            implicitHeight: Math.round(26 * Metrics.scale)
            radius: height / 2
            color: row.checked ? Style.accent : Style.line
            Behavior on color { ColorAnimation { duration: Metrics.fadeDuration } }

            Rectangle {
                width: parent.height - 6
                height: width
                radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                x: row.checked ? parent.width - width - 3 : 3
                color: "#ffffff"
                Behavior on x { NumberAnimation { duration: Metrics.fadeDuration; easing.type: Easing.OutCubic } }
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

    TapHandler {
        id: tap
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: row.toggled(!row.checked)
    }
}
