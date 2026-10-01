import QtQuick
import QtQuick.Layouts

SettingsCard {
    id: card

    visible: PlacePage.open && (Routing.state === "idle" || Routing.pickTarget !== "")
    implicitHeight: body.implicitHeight + 2 * Metrics.spacing

    TapHandler {
        onTapped: AppShell.push("PlaceDetailsPage.qml")
    }

    RowLayout {
        id: body
        Layout.fillWidth: true
        Layout.leftMargin: card.sideInset
        Layout.rightMargin: card.sideInset
        Layout.topMargin: Metrics.spacing
        Layout.bottomMargin: Metrics.spacing
        spacing: Metrics.spacing

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Metrics.spacingTight

            Text {
                Layout.fillWidth: true
                text: PlacePage.title
                color: Style.text
                elide: Text.ElideRight
                font.pixelSize: Metrics.fontMedium
                font.weight: Font.DemiBold
            }

            Text {
                Layout.fillWidth: true
                visible: text !== ""
                text: [PlacePage.subtitle, PlacePage.distance].filter(s => s !== "").join(" · ")
                color: Style.textMuted
                elide: Text.ElideRight
                font.pixelSize: Metrics.fontSmall
            }

            Text {
                Layout.fillWidth: true
                visible: text !== "" && text !== PlacePage.title
                text: PlacePage.address
                color: Style.textDim
                elide: Text.ElideRight
                font.pixelSize: Metrics.fontSmall
            }
        }

        ActionButton {
            Layout.alignment: Qt.AlignVCenter
            visible: Routing.pickTarget !== ""
            primary: true
            label: Routing.pickTarget === "start" ? qsTr("Set as start") : qsTr("Set as destination")
            onClicked: Routing.useSelectionForPick()
        }

        AccentButton {
            Layout.alignment: Qt.AlignVCenter
            visible: Routing.pickTarget === "" && !PlacePage.isMyPosition
            round: true
            glyph: "directions"
            onTapped: Routing.routeToSelection()
        }
    }
}
