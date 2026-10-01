import QtQuick
import QtQuick.Layouts

SettingsCard {
    id: panel

    implicitHeight: body.implicitHeight + 2 * Metrics.spacing

    ColumnLayout {
        id: body
        Layout.fillWidth: true
        Layout.leftMargin: panel.sideInset
        Layout.rightMargin: panel.sideInset
        Layout.topMargin: Metrics.spacing
        Layout.bottomMargin: Metrics.spacing
        spacing: Metrics.spacing

        InfoBanner {
            visible: Routing.finished
            kind: "accent"
            glyph: "sports_score"
            text: qsTr("You have arrived at %1").arg(Routing.toName)
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.spacing

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.spacingTight

                Text {
                    Layout.fillWidth: true
                    text: Routing.arrival
                    color: Style.text
                    font.pixelSize: Metrics.fontHuge
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("%1 · %2").arg(Routing.totalTime).arg(Routing.totalDistance)
                    color: Style.textMuted
                    elide: Text.ElideRight
                    font.pixelSize: Metrics.fontSmall
                }
            }

            AccentButton {
                round: true
                baseColor: Style.danger
                litInk: "#ffffff"
                glyph: "close"
                onTapped: panel.confirmEnd()
            }
        }

        LevelBar {
            Layout.fillWidth: true
            from: 0
            to: 1
            value: Routing.completion
        }
    }

    function confirmEnd() {
        if (Routing.finished) {
            Routing.cancel()
            return
        }
        AppShell.confirm(qsTr("End navigation?"), "", qsTr("End"), true, () => Routing.cancel())
    }
}
