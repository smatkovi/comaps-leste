import QtQuick
import QtQuick.Layouts

CardPage {
    title: qsTr("Route options")
    icon: "tune"

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        text: qsTr("Avoid")
    }

    SettingsCard {
        Layout.fillWidth: true

        SettingsToggleRow {
            visible: Routing.routerType === Routing.Car
            glyph: "toll"
            label: qsTr("Toll roads")
            checked: Routing.avoidToll
            divider: true
            onToggled: function (on) { Routing.avoidToll = on }
        }

        SettingsToggleRow {
            visible: Routing.routerType === Routing.Car
            glyph: "add_road"
            label: qsTr("Motorways")
            checked: Routing.avoidMotorway
            divider: true
            onToggled: function (on) { Routing.avoidMotorway = on }
        }

        SettingsToggleRow {
            glyph: "directions_boat"
            label: qsTr("Ferries")
            checked: Routing.avoidFerry
            divider: true
            onToggled: function (on) { Routing.avoidFerry = on }
        }

        SettingsToggleRow {
            glyph: "landscape"
            label: qsTr("Unpaved roads")
            checked: Routing.avoidDirty
            divider: Routing.routerType !== Routing.Car
            onToggled: function (on) { Routing.avoidDirty = on }
        }

        SettingsToggleRow {
            visible: Routing.routerType !== Routing.Car
            glyph: "stairs"
            label: qsTr("Steps")
            checked: Routing.avoidSteps
            onToggled: function (on) { Routing.avoidSteps = on }
        }
    }
}
