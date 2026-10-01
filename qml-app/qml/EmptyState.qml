import QtQuick
import QtQuick.Layouts

// Shown where a list has nothing in it.
ColumnLayout {
    id: root

    property string glyph
    property string title
    property string hint

    Layout.fillWidth: true
    Layout.topMargin: Metrics.spacingLarge
    spacing: Metrics.spacingSmall

    AppIcon {
        Layout.alignment: Qt.AlignHCenter
        Layout.topMargin: Metrics.spacingLarge
        name: root.glyph
        size: Metrics.fontHuge * 2
        color: Style.textDim
    }

    Text {
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignHCenter
        text: root.title
        color: Style.text
        wrapMode: Text.Wrap
        font.pixelSize: Metrics.fontMedium
        font.weight: Font.DemiBold
    }

    Text {
        Layout.fillWidth: true
        visible: root.hint !== ""
        horizontalAlignment: Text.AlignHCenter
        text: root.hint
        color: Style.textDim
        wrapMode: Text.Wrap
        font.pixelSize: Metrics.fontSmall
    }
}
