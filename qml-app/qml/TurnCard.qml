import QtQuick
import QtQuick.Layouts

AccentFill {
    id: card

    implicitHeight: body.implicitHeight + 2 * Metrics.spacing
    radius: Metrics.radiusLarge

    ColumnLayout {
        id: body
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        spacing: Metrics.spacingSmall

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.spacing

            AppIcon {
                Layout.preferredWidth: size
                name: Routing.turnGlyph
                size: Metrics.fontHuge * 2
                color: Style.accentInk
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.spacingTight

                Text {
                    Layout.fillWidth: true
                    text: Routing.distanceToTurn
                    color: Style.accentInk
                    font.pixelSize: Metrics.fontHuge
                    font.weight: Font.Bold
                }

                Text {
                    Layout.fillWidth: true
                    text: Routing.nextStreet !== "" ? Routing.nextStreet : Routing.turnText
                    color: Style.accentInk
                    elide: Text.ElideRight
                    maximumLineCount: 2
                    wrapMode: Text.Wrap
                    font.pixelSize: Metrics.fontMedium
                    font.weight: Font.Medium
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            visible: Routing.thenGlyph !== ""
            color: Style.accentInk
            opacity: 0.25
        }

        RowLayout {
            visible: Routing.thenGlyph !== ""
            spacing: Metrics.spacingSmall

            Text {
                text: qsTr("Then")
                color: Style.accentInk
                font.pixelSize: Metrics.fontSmall
            }

            AppIcon {
                name: Routing.thenGlyph
                size: Metrics.fontLarge
                color: Style.accentInk
            }
        }
    }
}
