import QtQuick

// A sheet that slides up from the bottom over a dimmed backdrop. Children are stacked in a column
// bodyWidth wide; tapping the backdrop closes it.
Item {
    id: sheet

    default property alias content: body.data
    property bool shown: false
    readonly property real bodyWidth: card.width - 2 * Metrics.pageMargin
    readonly property real maxCardHeight: sheet.height * 0.85

    function open() { sheet.shown = true }
    function close() { sheet.shown = false }

    anchors.fill: parent
    z: 900
    visible: scrim.opacity > 0

    Rectangle {
        id: scrim
        anchors.fill: parent
        color: "black"
        opacity: sheet.shown ? 0.4 : 0
        Behavior on opacity { NumberAnimation { duration: Metrics.fadeDuration } }

        TapHandler {
            onTapped: sheet.close()
        }
    }

    Rectangle {
        id: card
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(sheet.width, 560 * Metrics.scale)
        height: Math.min(sheet.maxCardHeight, body.implicitHeight + Metrics.spacingLarge + Metrics.spacing + Metrics.spacingLarge)
        y: sheet.shown ? sheet.height - height : sheet.height
        radius: Metrics.radiusSheet
        color: Metrics.background
        Behavior on y { NumberAnimation { duration: 220; easing.type: Easing.OutCubic } }

        // Keeps taps on the sheet from reaching the backdrop or the map.
        MouseArea {
            anchors.fill: parent
        }

        // Square bottom corners: the sheet sits on the bottom edge.
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: parent.radius
            color: parent.color
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: parent.top
            anchors.topMargin: Metrics.spacingSmall
            width: Math.round(36 * Metrics.scale)
            height: 4
            radius: 2
            color: Style.line
        }

        Column {
            id: body
            anchors.top: parent.top
            anchors.topMargin: Metrics.spacingLarge + Metrics.spacing
            anchors.horizontalCenter: parent.horizontalCenter
            width: sheet.bodyWidth
            spacing: Metrics.spacing
        }
    }
}
