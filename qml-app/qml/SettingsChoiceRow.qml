import QtQuick
import QtQuick.Layouts

// One choice of several: tapping it activates it, the selected one is ticked. With detail set, a
// chevron says there is more behind it.
Item {
    id: row

    property string label
    property bool detail: false
    property bool selected: false
    property bool divider: false

    signal activated()

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

        Text {
            Layout.fillWidth: true
            text: row.label
            color: Style.text
            elide: Text.ElideRight
            font.pixelSize: Metrics.fontNormal
        }

        AppIcon {
            visible: row.selected
            name: "check"
            color: Style.accentOnSurface
        }

        AppIcon {
            visible: row.detail
            name: "chevron_right"
            color: Style.textDim
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Metrics.sideInset
        height: 1
        visible: row.divider
        color: Style.line
    }

    TapHandler {
        id: tap
        gesturePolicy: TapHandler.ReleaseWithinBounds
        onTapped: row.activated()
    }
}
