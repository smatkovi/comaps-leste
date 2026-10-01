import QtQuick

// A short status in a pill; kind "accent" or "warn" colours its dot and border.
Rectangle {
    id: root

    property string text
    property string kind
    property bool showDot: true

    readonly property color tone: root.kind === "warn" ? Style.warn
                                : root.kind === "accent" ? Style.accentOnSurface
                                : Style.textDim

    implicitWidth: row.implicitWidth + 2 * Metrics.spacing
    implicitHeight: Math.max(Math.round(30 * Metrics.scale), row.implicitHeight + Metrics.spacingSmall)
    radius: height / 2
    color: Style.surfaceRaised
    border.width: 1
    border.color: root.kind === "" ? Style.line : root.tone

    Row {
        id: row
        anchors.centerIn: parent
        spacing: Metrics.spacingSmall

        Rectangle {
            anchors.verticalCenter: parent.verticalCenter
            visible: root.showDot
            width: Math.round(8 * Metrics.scale)
            height: width
            radius: width / 2
            color: root.tone
        }

        Text {
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: Style.text
            font.pixelSize: Metrics.fontSmall
            font.weight: Font.DemiBold
        }
    }
}
