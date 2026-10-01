import QtQuick
import QtQuick.Layouts

// Sections of categories ({title, items: [{key, icon, label}]}) to search for; scrolls by itself.
Flickable {
    id: root

    property var categories: []

    signal picked(string key)

    contentWidth: width
    contentHeight: column.implicitHeight + Metrics.spacing
    boundsBehavior: Flickable.StopAtBounds
    clip: true
    interactive: contentHeight > height

    ColumnLayout {
        id: column
        width: root.width
        spacing: Metrics.spacing

        Repeater {
            model: root.categories

            delegate: ColumnLayout {
                id: section

                required property var modelData

                Layout.fillWidth: true
                spacing: Metrics.spacingSmall

                SettingsSubHeader {
                    Layout.leftMargin: Metrics.sideInset
                    text: section.modelData.title
                }

                SettingsCard {
                    Layout.fillWidth: true

                    Repeater {
                        model: section.modelData.items

                        delegate: SettingsNavRow {
                            required property var modelData
                            required property int index

                            glyph: modelData.icon
                            label: modelData.label
                            chevron: false
                            divider: index < section.modelData.items.length - 1
                            onTapped: root.picked(modelData.key)
                        }
                    }
                }
            }
        }
    }
}
