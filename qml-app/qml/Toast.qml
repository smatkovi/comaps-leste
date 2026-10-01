import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property bool isError: false

    implicitHeight: row.implicitHeight + 2 * Metrics.spacing
    radius: height / 2
    color: Style.surfaceRaised
    border.width: 1
    border.color: root.isError ? Style.danger : Style.line
    opacity: 0
    visible: opacity > 0

    RowLayout {
        id: row
        anchors.fill: parent
        anchors.leftMargin: Metrics.spacingLarge * 0.75
        anchors.rightMargin: Metrics.spacingLarge * 0.75
        spacing: Metrics.spacingSmall

        AppIcon {
            name: root.isError ? "error_outline" : "info_outline"
            color: root.isError ? Style.danger : Style.accentOnSurface
            size: Metrics.fontLarge
        }

        Text {
            id: label
            Layout.fillWidth: true
            color: Style.text
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
            font.pixelSize: Metrics.fontSmall
        }
    }

    Timer {
        id: hideTimer
        interval: 3200
        onTriggered: root.opacity = 0
    }

    Behavior on opacity { NumberAnimation { duration: Metrics.fadeDuration } }

    TapHandler { onTapped: root.opacity = 0 }

    function show(text, error) {
        if (!text)
            return
        label.text = text
        root.isError = error === true
        root.opacity = 1
        hideTimer.restart()
    }
}
