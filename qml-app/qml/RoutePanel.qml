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
            visible: Routing.pickTarget !== ""
            kind: "accent"
            glyph: "touch_app"
            text: Routing.pickTarget === "start"
                  ? qsTr("Tap a place on the map, or press and hold anywhere, to set the start point.")
                  : qsTr("Tap a place on the map, or press and hold anywhere, to set the destination.")
        }

        RowLayout {
            Layout.fillWidth: true
            visible: Routing.pickTarget !== ""
            spacing: Metrics.spacing

            ActionButton {
                label: qsTr("Cancel")
                onClicked: Routing.pickTarget = ""
            }

            ActionButton {
                Layout.fillWidth: true
                glyph: "search"
                label: qsTr("Search instead")
                onClicked: AppShell.openSearch()
            }
        }

        StatusPill {
            visible: Routing.pickTarget === "" && Routing.building
            kind: "accent"
            text: qsTr("Finding the best route…")
        }

        InfoBanner {
            visible: Routing.pickTarget === "" && !Routing.building && Routing.error !== ""
            kind: "warn"
            text: Routing.missingMaps.length > 0
                  ? Routing.error + ": " + Routing.missingMaps.join(", ")
                  : Routing.error
        }

        ActionButton {
            Layout.fillWidth: true
            visible: Routing.pickTarget === "" && !Routing.building && Routing.missingMaps.length > 0
            primary: true
            glyph: "download"
            label: qsTr("Download maps")
            onClicked: {
                Routing.downloadMissingMaps()
                AppShell.openDownloader("")
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            visible: Routing.pickTarget === "" && Routing.ready && !Routing.building
            spacing: Metrics.spacingTight

            Text {
                Layout.fillWidth: true
                text: Routing.totalTime
                color: Style.text
                font.pixelSize: Metrics.fontHuge
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("%1 · arrive at %2").arg(Routing.totalDistance).arg(Routing.arrival)
                color: Style.textMuted
                elide: Text.ElideRight
                font.pixelSize: Metrics.fontSmall
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: Routing.pickTarget === ""
            spacing: Metrics.spacing

            ActionButton {
                glyph: "tune"
                label: qsTr("Options")
                onClicked: Windows.openRouteOptions()
            }

            ActionButton {
                Layout.fillWidth: true
                primary: true
                enabled: Routing.ready && !Routing.building
                glyph: "navigation"
                label: qsTr("Start")
                onClicked: Routing.start()
            }
        }
    }
}
