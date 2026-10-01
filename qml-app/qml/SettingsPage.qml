import QtQuick
import QtQuick.Layouts

CardPage {
    title: qsTr("Settings")
    icon: "settings"

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        text: qsTr("Units")
    }

    OptionSelector {
        Layout.fillWidth: true
        current: String(Settings.units)
        options: [
            { key: "0", label: qsTr("Metric") },
            { key: "1", label: qsTr("Imperial") }
        ]
        onSelected: function (key) { Settings.units = Number(key) }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Pinch zoom speed")
    }

    OptionSelector {
        Layout.fillWidth: true
        current: String(Settings.pinchSpeed)
        // 0: 1.6× up to zoom 16, 1.3× closer in.
        options: [
            { key: "0", label: qsTr("Dynamic") },
            { key: "1", label: qsTr("1×") },
            { key: "1.3", label: qsTr("1.3×") },
            { key: "1.6", label: qsTr("1.6×") },
            { key: "2", label: qsTr("2×") }
        ]
        onSelected: function (key) { Settings.pinchSpeed = Number(key) }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Scroll speed")
    }

    OptionSelector {
        Layout.fillWidth: true
        current: String(Settings.scrollSpeed)
        // 0: 1.6× up to zoom 16, 1.3× closer in.
        options: [
            { key: "0", label: qsTr("Dynamic") },
            { key: "1", label: qsTr("1×") },
            { key: "1.3", label: qsTr("1.3×") },
            { key: "1.6", label: qsTr("1.6×") },
            { key: "2", label: qsTr("2×") }
        ]
        onSelected: function (key) { Settings.scrollSpeed = Number(key) }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Map icon size")
    }

    OptionSelector {
        Layout.fillWidth: true
        current: String(Settings.iconScale)
        options: [
            { key: "1", label: qsTr("1×") },
            { key: "1.25", label: qsTr("1.25×") },
            { key: "1.5", label: qsTr("1.5×") }
        ]
        onSelected: function (key) { Settings.iconScale = Number(key) }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Map label size")
    }

    OptionSelector {
        Layout.fillWidth: true
        current: String(Settings.labelScale)
        // 0: 1.6× up to zoom 16, 1.3× closer in.
        options: [
            { key: "0", label: qsTr("Dynamic") },
            { key: "1", label: qsTr("1×") },
            { key: "1.3", label: qsTr("1.3×") },
            { key: "1.6", label: qsTr("1.6×") },
            { key: "2", label: qsTr("2×") }
        ]
        onSelected: function (key) { Settings.labelScale = Number(key) }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Map")
    }

    SettingsCard {
        Layout.fillWidth: true

        SettingsToggleRow {
            glyph: "apartment"
            label: qsTr("3D buildings")
            checked: MapController.buildings3d
            divider: true
            onToggled: function (on) { MapController.buildings3d = on }
        }

        SettingsToggleRow {
            glyph: "3d_rotation"
            label: qsTr("Perspective view")
            description: qsTr("Tilt the map when zoomed in")
            checked: Settings.perspective
            divider: true
            onToggled: function (on) { Settings.perspective = on }
        }

        SettingsToggleRow {
            glyph: "bookmark"
            label: qsTr("Bookmark names")
            checked: Settings.bookmarkLabels
            divider: true
            onToggled: function (on) { Settings.bookmarkLabels = on }
        }

        SettingsToggleRow {
            glyph: "translate"
            label: qsTr("Transliterate names")
            description: qsTr("Show names in Latin script")
            checked: Settings.transliteration
            onToggled: function (on) { Settings.transliteration = on }
        }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Map language")
    }

    SettingsCard {
        Layout.fillWidth: true

        SettingsChoiceRow {
            label: qsTr("Automatic")
            detail: false
            selected: Settings.mapLanguage === "auto"
            divider: true
            onActivated: Settings.mapLanguage = "auto"
        }

        SettingsChoiceRow {
            label: qsTr("Local names")
            detail: false
            selected: Settings.mapLanguage === "local"
            divider: true
            onActivated: Settings.mapLanguage = "local"
        }

        SettingsChoiceRow {
            label: qsTr("English")
            detail: false
            selected: Settings.mapLanguage === "en"
            onActivated: Settings.mapLanguage = "en"
        }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("Navigation")
    }

    SettingsCard {
        Layout.fillWidth: true

        SettingsToggleRow {
            glyph: "3d_rotation"
            label: qsTr("Tilt while navigating")
            description: qsTr("Switch to perspective when navigation starts")
            checked: Settings.navigationPerspective
            divider: true
            onToggled: function (on) { Settings.navigationPerspective = on }
        }

        SettingsToggleRow {
            glyph: "zoom_in_map"
            label: qsTr("Auto zoom")
            description: qsTr("Zoom in before turns")
            checked: Settings.autoZoom
            divider: true
            onToggled: function (on) { Settings.autoZoom = on }
        }

        SettingsNavRow {
            glyph: "tune"
            label: qsTr("Route options")
            onTapped: AppShell.push("RouteOptionsPage.qml")
        }
    }

    SettingsSubHeader {
        Layout.leftMargin: Metrics.sideInset
        Layout.topMargin: Metrics.spacingSmall
        text: qsTr("More")
    }

    SettingsCard {
        Layout.fillWidth: true

        SettingsNavRow {
            glyph: "map"
            label: qsTr("Map downloads")
            divider: true
            onTapped: AppShell.openDownloader("")
        }

        SettingsNavRow {
            glyph: "info_outline"
            label: qsTr("About")
            value: Settings.version
            onTapped: AppShell.openAbout()
        }
    }
}
