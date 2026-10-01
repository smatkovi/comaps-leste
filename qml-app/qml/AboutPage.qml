import QtQuick
import QtQuick.Layouts

CardPage {
    title: qsTr("About")
    icon: "info_outline"

    SettingsCard {
        Layout.fillWidth: true

        SettingsValueRow {
            glyph: "map"
            label: qsTr("Maps")
            value: Settings.version
            divider: true
        }

        SettingsValueRow {
            glyph: "public"
            label: qsTr("Map data")
            value: qsTr("© OpenStreetMap contributors")
            stacked: true
        }
    }

    SettingsCard {
        Layout.fillWidth: true
        implicitHeight: licence.implicitHeight + 2 * Metrics.spacing

        Text {
            id: licence
            Layout.fillWidth: true
            Layout.leftMargin: Metrics.sideInset
            Layout.rightMargin: Metrics.sideInset
            Layout.topMargin: Metrics.spacing
            Layout.bottomMargin: Metrics.spacing
            wrapMode: Text.Wrap
            color: Style.textMuted
            font.pixelSize: Metrics.fontSmall
            lineHeight: 1.2
            text: qsTr("Maps is built on the CoMaps engine, free and open source software licensed under the Apache License 2.0. "
                       + "Map data is © OpenStreetMap contributors and available under the Open Database License (ODbL). "
                       + "Maps works offline: downloaded maps stay on this device, and routes and searches never leave it.")
        }
    }
}
