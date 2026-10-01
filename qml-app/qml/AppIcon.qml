import QtQuick

Text {
    id: root

    property string name
    property int size: Metrics.iconSize

    text: Icons.glyph(root.name)
    font.family: Icons.family
    font.pixelSize: root.size
    color: Style.text
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
}
