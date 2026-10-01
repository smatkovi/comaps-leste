import QtQuick
import QtQuick.Layouts

CardPage {
    id: page

    title: PlacePage.title
    icon: PlacePage.isBookmark ? "bookmark" : "place"

    Connections {
        target: PlacePage
        function onOpenChanged() {
            if (!PlacePage.open)
                AppShell.popToMap()
        }
    }

    Text {
        Layout.fillWidth: true
        visible: text !== ""
        text: [PlacePage.subtitle, PlacePage.distance].filter(s => s !== "").join(" · ")
        color: Style.textMuted
        wrapMode: Text.Wrap
        font.pixelSize: Metrics.fontNormal
    }

    InfoBanner {
        visible: PlacePage.openState !== ""
        kind: PlacePage.openState === "open" ? "accent" : "danger"
        glyph: "schedule"
        text: (PlacePage.openState === "open" ? qsTr("Open now") : qsTr("Closed now")) + " · " + PlacePage.openingHours
    }

    SettingsCard {
        Layout.fillWidth: true
        visible: !PlacePage.isMyPosition

        CardAction {
            glyph: "directions"
            label: qsTr("Route here")
            divider: true
            onTapped: {
                Routing.routeToSelection()
                AppShell.popToMap()
            }
        }

        CardAction {
            glyph: "trip_origin"
            label: qsTr("Route from here")
            divider: true
            onTapped: {
                Routing.routeFromSelection()
                AppShell.popToMap()
            }
        }

        CardAction {
            glyph: PlacePage.isBookmark ? "bookmark" : "bookmark_border"
            label: PlacePage.isBookmark ? qsTr("Remove bookmark") : qsTr("Save as bookmark")
            onTapped: PlacePage.toggleBookmark()
        }
    }

    SettingsCard {
        Layout.fillWidth: true
        visible: details.count > 0

        Repeater {
            id: details

            model: [
                { glyph: "place", label: qsTr("Address"), value: PlacePage.address },
                { glyph: "schedule", label: qsTr("Opening hours"), value: PlacePage.openState === "" ? PlacePage.openingHours : "" },
                { glyph: "phone", label: qsTr("Phone"), value: PlacePage.phone },
                { glyph: "language", label: qsTr("Website"), value: PlacePage.website },
                { glyph: "email", label: qsTr("Email"), value: PlacePage.email },
                { glyph: "restaurant", label: qsTr("Cuisine"), value: PlacePage.cuisine },
                { glyph: "business", label: qsTr("Operator"), value: PlacePage.operatorName },
                { glyph: "layers", label: qsTr("Level"), value: PlacePage.level },
                { glyph: "wifi", label: qsTr("Wi-Fi"), value: PlacePage.wifi ? qsTr("Available") : "" },
                { glyph: "bookmark", label: qsTr("Bookmark list"), value: PlacePage.bookmarkList },
                { glyph: "my_location", label: qsTr("Coordinates"), value: PlacePage.coordinates }
            ].filter(entry => entry.value !== "")

            delegate: SettingsValueRow {
                required property var modelData
                required property int index

                glyph: modelData.glyph
                label: modelData.label
                value: modelData.value
                stacked: true
                divider: index < details.count - 1
            }
        }
    }

    SettingsCard {
        Layout.fillWidth: true
        visible: PlacePage.description !== ""
        implicitHeight: description.implicitHeight + 2 * Metrics.spacing

        Text {
            id: description
            Layout.fillWidth: true
            Layout.leftMargin: Metrics.sideInset
            Layout.rightMargin: Metrics.sideInset
            Layout.topMargin: Metrics.spacing
            Layout.bottomMargin: Metrics.spacing
            text: PlacePage.description
            color: Style.text
            wrapMode: Text.Wrap
            maximumLineCount: 12
            elide: Text.ElideRight
            font.pixelSize: Metrics.fontSmall
            lineHeight: 1.2
        }
    }
}
