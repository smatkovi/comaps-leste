import QtQuick
import QtQuick.Layouts

SettingsCard {
    id: card

    readonly property string status: MapController.countryStatus
    readonly property bool busy: status === "downloading" || status === "queued"

    readonly property bool showDownload: MapController.countryId !== "" && (status === "notDownloaded" || status === "error" || busy)
    implicitHeight: body.implicitHeight + 2 * Metrics.spacing

    ColumnLayout {
        id: body
        Layout.fillWidth: true
        Layout.leftMargin: card.sideInset
        Layout.rightMargin: card.sideInset
        Layout.topMargin: Metrics.spacing
        Layout.bottomMargin: Metrics.spacing
        spacing: Metrics.spacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.spacing

            AccentIconTile {
                glyph: card.status === "error" ? "error_outline" : "map"
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Metrics.spacingTight

                Text {
                    Layout.fillWidth: true
                    text: MapController.countryName
                    color: Style.text
                    elide: Text.ElideRight
                    font.pixelSize: Metrics.fontNormal
                    font.weight: Font.Medium
                }

                Text {
                    Layout.fillWidth: true
                    text: card.status === "error" ? qsTr("Download failed")
                        : card.status === "queued" ? qsTr("Waiting to download")
                        : card.status === "downloading" ? qsTr("Downloading %1%").arg(Math.round(MapController.countryProgress * 100))
                        : MapController.countrySize
                    color: card.status === "error" ? Style.danger : Style.textDim
                    elide: Text.ElideRight
                    font.pixelSize: Metrics.fontSmall
                }
            }

            ActionButton {
                primary: card.status === "notDownloaded"
                label: card.busy ? qsTr("Cancel") : card.status === "error" ? qsTr("Retry") : qsTr("Download")
                onClicked: {
                    if (card.busy)
                        MapController.cancelCountry()
                    else if (card.status === "error")
                        MapController.retryCountry()
                    else
                        MapController.downloadCountry()
                }
            }
        }

        LevelBar {
            Layout.fillWidth: true
            visible: card.busy
            from: 0
            to: 1
            value: MapController.countryProgress
        }
    }
}
