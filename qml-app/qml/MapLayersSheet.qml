import QtQuick
import QtQuick.Layouts

SheetFrame {
    id: sheet

    readonly property int hidden: MapController.hiddenFeatures

    function shown_(group) { return (sheet.hidden & group) === 0 }
    function setShown(group, on) {
        MapController.hiddenFeatures = on ? (sheet.hidden & ~group) : (sheet.hidden | group)
    }

    Flickable {
        id: flick
        width: sheet.bodyWidth
        height: Math.min(column.implicitHeight, sheet.maxCardHeight - Style.scaleFactor * 46)
        contentWidth: width
        contentHeight: column.implicitHeight
        boundsBehavior: Flickable.StopAtBounds
        clip: true
        interactive: contentHeight > height

        ColumnLayout {
            id: column
            width: flick.width
            spacing: Metrics.spacing

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Map layers")
                color: Style.text
                font.pixelSize: Metrics.fontLarge
                font.weight: Font.DemiBold
            }

            SettingsCard {
                Layout.fillWidth: true

                SettingsToggleRow {
                    glyph: "landscape"
                    label: qsTr("Outdoors")
                    checked: MapController.outdoor
                    divider: true
                    onToggled: function (on) { MapController.outdoor = on }
                }

                SettingsToggleRow {
                    glyph: "terrain"
                    label: qsTr("Contour lines")
                    checked: MapController.contourLines
                    divider: true
                    onToggled: function (on) { MapController.contourLines = on }
                }

                SettingsToggleRow {
                    glyph: "domain"
                    label: qsTr("Buildings")
                    checked: sheet.shown_(MapController.HideBuildings)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideBuildings, on) }
                }

                SettingsToggleRow {
                    glyph: "apartment"
                    label: qsTr("3D buildings")
                    enabled: sheet.shown_(MapController.HideBuildings)
                    checked: MapController.buildings3d
                    divider: true
                    onToggled: function (on) { MapController.buildings3d = on }
                }

                SettingsToggleRow {
                    glyph: "subway"
                    label: qsTr("Public transport")
                    checked: MapController.mapMode === MapController.PublicTransport
                    onToggled: function (on) {
                        MapController.mapMode = on ? MapController.PublicTransport : MapController.DefaultMode
                    }
                }
            }

            SettingsSubHeader {
                Layout.leftMargin: Metrics.sideInset
                Layout.topMargin: Metrics.spacingSmall
                text: qsTr("Show on the map")
            }

            SettingsCard {
                Layout.fillWidth: true

                SettingsToggleRow {
                    glyph: "restaurant"
                    label: qsTr("Restaurants, cafés and bars")
                    checked: sheet.shown_(MapController.HideFood)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideFood, on) }
                }

                SettingsToggleRow {
                    glyph: "local_grocery_store"
                    label: qsTr("Shops")
                    checked: sheet.shown_(MapController.HideShops)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideShops, on) }
                }

                SettingsToggleRow {
                    glyph: "hotel"
                    label: qsTr("Hotels and sights")
                    checked: sheet.shown_(MapController.HideTourism)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideTourism, on) }
                }

                SettingsToggleRow {
                    glyph: "local_atm"
                    label: qsTr("Other places")
                    description: qsTr("Banks, parking, offices, doctors, gyms…")
                    checked: sheet.shown_(MapController.HideServices)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideServices, on) }
                }

                SettingsToggleRow {
                    glyph: "pin"
                    label: qsTr("House numbers")
                    checked: sheet.shown_(MapController.HideHouseNumbers)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideHouseNumbers, on) }
                }

                SettingsToggleRow {
                    glyph: "door_front"
                    label: qsTr("Entrances")
                    checked: sheet.shown_(MapController.HideEntrances)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideEntrances, on) }
                }

                SettingsToggleRow {
                    glyph: "elevator"
                    label: qsTr("Elevators")
                    checked: sheet.shown_(MapController.HideElevators)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideElevators, on) }
                }

                SettingsToggleRow {
                    glyph: "fence"
                    label: qsTr("Barriers and gates")
                    description: qsTr("Gates, bollards, stiles, turnstiles…")
                    checked: sheet.shown_(MapController.HideBarriers)
                    divider: true
                    onToggled: function (on) { sheet.setShown(MapController.HideBarriers, on) }
                }

                SettingsToggleRow {
                    glyph: "delete"
                    label: qsTr("Dumpsters and bins")
                    checked: sheet.shown_(MapController.HideWaste)
                    onToggled: function (on) { sheet.setShown(MapController.HideWaste, on) }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: Metrics.spacingSmall

                ActionButton {
                    Layout.fillWidth: true
                    enabled: sheet.hidden !== 0
                    glyph: "visibility"
                    label: qsTr("Show everything")
                    onClicked: MapController.hiddenFeatures = 0
                }

                ActionButton {
                    Layout.fillWidth: true
                    enabled: (sheet.hidden & MapController.allHiddenGroups) !== MapController.allHiddenGroups
                    glyph: "visibility_off"
                    label: qsTr("Hide everything")
                    onClicked: MapController.hiddenFeatures = sheet.hidden | MapController.allHiddenGroups
                }
            }
        }
    }
}
