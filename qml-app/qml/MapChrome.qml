import QtQuick
import QtQuick.Layouts

Item {
    id: chrome

    readonly property string mode: Routing.state
    readonly property bool landscape: width > height
    readonly property real obstructedTop: topArea.y + topArea.height
    readonly property real obstructedBottom: bottomArea.y

    property string title: qsTr("Map")
    readonly property string onMapSearchLabel:
        MapSearch.searching ? MapSearch.label
      : MapSearch.count > 0 ? qsTr("%1 (%2)").arg(MapSearch.label).arg(MapSearch.count)
      : qsTr("%1 · nothing found").arg(MapSearch.label)
    readonly property var pageActions: mode === "planning" ? [
        { text: qsTr("Route options"), trigger: () => AppShell.push("RouteOptionsPage.qml") },
        { text: qsTr("Close route"), trigger: () => Routing.cancel() }
    ] : []

    Item {
        id: topArea
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Metrics.spacing
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        height: chrome.mode === "navigating" ? turnCard.height
              : chrome.mode === "planning" ? routeCard.height
              : searchRow.visible ? searchRow.height : 0

        RowLayout {
            id: searchRow
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            visible: chrome.mode === "idle" && (AppShell.mapSearchShown || MapSearch.active)
            spacing: Metrics.spacingSmall

            // Typing hands over to the search page; while a search is shown on the map, the bar says
            // what is shown. Its close button leaves search mode.
            SearchBox {
                id: search
                Layout.fillWidth: true
                closable: true
                placeholderText: MapSearch.active ? chrome.onMapSearchLabel : qsTr("Search Maps")
                border.width: 1
                border.color: MapSearch.active ? Style.accent : Style.line
                onEdited: function (value) {
                    if (value === "")
                        return
                    AppShell.mapSearchShown = false
                    AppShell.openSearch(value)
                    Qt.callLater(() => search.text = "")
                }
                onCloseRequested: chrome.closeSearch()
            }
        }

        Connections {
            target: AppShell
            function onMapSearchRequested() { Qt.callLater(search.focusInput) }
        }

        RouteCard {
            id: routeCard
            anchors.left: parent.left
            anchors.right: parent.right
            visible: chrome.mode === "planning"
        }

        TurnCard {
            id: turnCard
            anchors.left: parent.left
            anchors.right: parent.right
            visible: chrome.mode === "navigating"
        }
    }

    // Portrait: a column on the right below the top bar. Landscape has no height for that, so the
    // buttons form a row on the position button's line, to its left.
    Grid {
        id: mapButtons
        columns: chrome.landscape ? children.length : 1
        x: chrome.landscape ? positionButton.x - width - Metrics.spacing : chrome.width - width - Metrics.pageMargin
        y: chrome.landscape ? positionButton.y + (positionButton.height - height) / 2
                            : Math.max(topArea.y + topArea.height + Metrics.spacingLarge, (chrome.height - height) / 2)
        visible: chrome.landscape ? x >= Metrics.pageMargin : y + height + Metrics.spacingLarge < positionButton.y
        spacing: Metrics.spacing

        MapButton {
            readonly property var modes: [
                { glyph: "directions_car", text: qsTr("Roads") },
                { glyph: "directions_walk", text: qsTr("Footpaths") },
                { glyph: "commute", text: qsTr("Roads and footpaths") }
            ]
            glyph: modes[MapController.roadMode].glyph
            onTapped: {
                MapController.roadMode = (MapController.roadMode + 1) % modes.length
                AppShell.toast(modes[MapController.roadMode].text)
            }
        }

        MapButton {
            glyph: "3d_rotation"
            lit: Settings.perspective
            onTapped: Settings.perspective = !Settings.perspective
        }

        MapButton {
            glyph: "layers"
            onTapped: AppShell.layersSheet.open()
        }

        Item {
            width: chrome.landscape ? Metrics.spacingSmall : 1
            height: chrome.landscape ? 1 : Metrics.spacingSmall
        }

        MapButton {
            glyph: "add"
            onTapped: MapController.zoomIn()
        }

        MapButton {
            glyph: "remove"
            onTapped: MapController.zoomOut()
        }
    }

    PositionButton {
        id: positionButton
        anchors.right: parent.right
        anchors.rightMargin: Metrics.pageMargin
        anchors.bottom: bottomArea.top
        anchors.bottomMargin: Metrics.spacing
        visible: chrome.mode !== "navigating" || MapController.myPositionMode !== MapController.FollowAndRotate
    }

    ColumnLayout {
        id: bottomArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        anchors.bottomMargin: Metrics.spacingLarge
        spacing: Metrics.spacing

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            visible: chrome.mode === "navigating" && (Routing.currentStreet !== "" || Routing.speedLimit !== "")
            spacing: Metrics.spacingSmall

            StatusPill {
                visible: Routing.currentStreet !== ""
                showDot: false
                text: Routing.currentStreet
            }

            StatusPill {
                visible: Routing.speedLimit !== ""
                kind: "warn"
                showDot: false
                text: Routing.speedLimit
            }
        }

        PlaceCard {
            Layout.fillWidth: true
        }

        DownloadCard {
            Layout.fillWidth: true
            visible: chrome.mode === "idle" && showDownload
        }

        RoutePanel {
            Layout.fillWidth: true
            visible: chrome.mode === "planning" && !(Routing.pickTarget !== "" && PlacePage.open)
        }

        NavigationPanel {
            id: navigationPanel
            Layout.fillWidth: true
            visible: chrome.mode === "navigating"
        }
    }

    function closeSearch() {
        search.text = ""
        AppShell.mapSearchShown = false
        if (MapSearch.active)
            MapSearch.clear()
    }

    function back() {
        if (Routing.pickTarget !== "" && !PlacePage.open) {
            Routing.pickTarget = ""
            return
        }
        if (PlacePage.open) {
            PlacePage.close()
            return
        }
        if (mode === "idle" && (AppShell.mapSearchShown || MapSearch.active)) {
            closeSearch()
            return
        }
        if (mode === "planning") {
            Routing.cancel()
            return
        }
        if (mode === "navigating")
            navigationPanel.confirmEnd()
    }
}
