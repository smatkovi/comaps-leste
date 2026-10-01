import QtQuick
import QtQuick.Layouts

CardPage {
    id: page

    property string parentId: ""

    title: parentId === "" ? qsTr("Map downloads") : countries.parentName
    icon: "map"

    CountryTreeModel {
        id: countries
        parentId: page.parentId
    }

    SearchBox {
        Layout.fillWidth: true
        placeholderText: qsTr("Search maps")
        onEdited: function (value) { countries.filter = value }
        onAccepted: Qt.inputMethod.hide()
    }

    InfoBanner {
        visible: page.parentId === "" && countries.updateCount > 0
        kind: "accent"
        glyph: "update"
        text: qsTr("%n map(s) can be updated (%1)", "", countries.updateCount).arg(countries.updateSize)
    }

    ActionButton {
        Layout.fillWidth: true
        visible: page.parentId === "" && countries.updateCount > 0
        primary: true
        glyph: "update"
        label: qsTr("Update all")
        onClicked: countries.updateAll()
    }

    SettingsCard {
        Layout.fillWidth: true
        visible: countries.count > 0

        Repeater {
            model: countries

            delegate: SettingsNavRow {
                id: row

                required property int index
                required property string countryId
                required property string name
                required property string status
                required property string size
                required property string detail
                required property real progress
                required property bool hasChildren

                readonly property bool busy: status === "downloading" || status === "queued"

                label: name
                value: page.statusText(status, detail, size, progress)
                chevron: hasChildren
                divider: index < countries.count - 1

                LevelBar {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: row.busy
                    width: 64 * Metrics.scale
                    from: 0
                    to: 1
                    value: row.progress
                }

                AppIcon {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: !row.busy && !row.hasChildren
                    name: row.status === "onDisk" ? "check_circle"
                        : row.status === "outdated" ? "update"
                        : row.status === "error" ? "error_outline"
                        : "download"
                    color: row.status === "error" ? Style.danger
                         : row.status === "notDownloaded" ? Style.textDim
                         : Style.accentOnSurface
                }

                IconButton {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: row.hasChildren
                    iconName: "more_vert"
                    onClicked: page.showActions(row.countryId, row.name, row.status, row.size)
                }

                onTapped: {
                    if (hasChildren)
                        AppShell.openDownloader(countryId)
                    else
                        page.showActions(countryId, name, status, size)
                }
            }
        }
    }

    function statusText(status, detail, size, progress) {
        switch (status) {
        case "downloading": return qsTr("Downloading %1%").arg(Math.round(progress * 100))
        case "queued": return qsTr("Waiting to download")
        case "error": return qsTr("Download failed")
        case "outdated": return qsTr("Update available · %1").arg(size)
        default: return detail
        }
    }

    function showOnMap(countryId) {
        MapController.showCountry(countryId)
        AppShell.popToMap()
    }

    function confirmDelete(countryId, name) {
        AppShell.confirm(qsTr("Delete %1?").arg(name), qsTr("The map will have to be downloaded again to use it offline."),
                         qsTr("Delete"), true, () => countries.remove(countryId))
    }

    function showActions(countryId, name, status, size) {
        const busy = status === "downloading" || status === "queued"
        const local = status === "onDisk" || status === "outdated" || status === "partly"
        AppShell.sheet(name, "", [
            { label: qsTr("Download (%1)").arg(size), glyph: "download",
              visible: status === "notDownloaded" || status === "partly",
              action: () => countries.download(countryId) },
            { label: qsTr("Update"), glyph: "update", visible: status === "outdated",
              action: () => countries.update(countryId) },
            { label: qsTr("Retry download"), glyph: "refresh", visible: status === "error",
              action: () => countries.retry(countryId) },
            { label: qsTr("Cancel download"), glyph: "close", visible: busy || status === "error",
              action: () => countries.cancel(countryId) },
            { label: qsTr("Show on map"), glyph: "map",
              action: () => page.showOnMap(countryId) },
            { label: qsTr("Delete"), glyph: "delete", destructive: true, visible: local && !busy,
              action: () => page.confirmDelete(countryId, name) }
        ])
    }
}
