import QtQuick
import QtQuick.Layouts

SettingsCard {
    id: card

    implicitHeight: body.implicitHeight + 2 * Metrics.spacing

    ColumnLayout {
        id: body
        Layout.fillWidth: true
        Layout.leftMargin: card.sideInset
        Layout.rightMargin: Metrics.spacingSmall
        Layout.topMargin: Metrics.spacing
        Layout.bottomMargin: Metrics.spacing
        spacing: Metrics.spacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Metrics.spacingSmall

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 0

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Metrics.buttonHeight
                    spacing: Metrics.spacing

                    TapHandler {
                        onTapped: card.choose(true)
                    }

                    AppIcon {
                        name: Routing.fromIsMyPosition ? "my_location" : "trip_origin"
                        color: Style.accentOnSurface
                    }

                    Text {
                        Layout.fillWidth: true
                        text: Routing.pickTarget === "start" ? qsTr("Choosing a start point…")
                            : Routing.fromName !== "" ? Routing.fromName : qsTr("Choose a start point")
                        color: Routing.fromName !== "" ? Style.text : Style.textDim
                        elide: Text.ElideRight
                        font.pixelSize: Metrics.fontNormal
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.leftMargin: Metrics.iconSize + Metrics.spacing
                    Layout.preferredHeight: 1
                    color: Style.line
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.preferredHeight: Metrics.buttonHeight
                    spacing: Metrics.spacing

                    TapHandler {
                        onTapped: card.choose(false)
                    }

                    AppIcon {
                        name: "place"
                        color: Style.danger
                    }

                    Text {
                        Layout.fillWidth: true
                        text: Routing.pickTarget === "finish" ? qsTr("Choosing a destination…")
                            : Routing.toName !== "" ? Routing.toName : qsTr("Choose a destination")
                        color: Routing.toName !== "" ? Style.text : Style.textDim
                        elide: Text.ElideRight
                        font.pixelSize: Metrics.fontNormal
                    }
                }
            }

            ColumnLayout {
                spacing: 0

                IconButton {
                    iconName: "close"
                    onClicked: Routing.cancel()
                }

                IconButton {
                    iconName: "swap_vert"
                    enabled: Routing.fromName !== "" && Routing.toName !== ""
                    onClicked: Routing.swap()
                }
            }
        }

        OptionSelector {
            Layout.fillWidth: true
            Layout.rightMargin: card.sideInset - Metrics.spacingSmall
            current: String(Routing.routerType)
            options: [
                { key: String(Routing.Car), label: qsTr("Car"), glyph: "directions_car" },
                { key: String(Routing.Walk), label: qsTr("Walk"), glyph: "directions_walk" },
                { key: String(Routing.Bike), label: qsTr("Bike"), glyph: "directions_bike" },
                { key: String(Routing.Transit), label: qsTr("Transit"), glyph: "directions_transit" }
            ]
            onSelected: function (key) { Routing.routerType = Number(key) }
        }
    }

    function choose(isStart) {
        const target = isStart ? "start" : "finish"
        AppShell.sheet(isStart ? qsTr("Start point") : qsTr("Destination"), "", [
            { label: qsTr("My position"), glyph: "my_location",
              action: () => Routing.useMyPosition(isStart) },
            { label: qsTr("Search for a place"), glyph: "search",
              action: () => {
                  Routing.pickTarget = target
                  AppShell.openSearch()
              } },
            { label: qsTr("Pick on the map"), glyph: "touch_app",
              action: () => {
                  PlacePage.close()
                  Routing.pickTarget = target
              } }
        ])
    }
}
