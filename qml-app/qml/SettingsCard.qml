import QtQuick
import QtQuick.Layouts

// A rounded group of rows. Children are stacked in a column; rows reach the card's edges and indent
// their content by sideInset.
Rectangle {
    id: card

    default property alias content: inner.data
    property int sideInset: Metrics.sideInset

    implicitHeight: inner.implicitHeight
    radius: Metrics.radius
    color: Style.surface

    ColumnLayout {
        id: inner
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 0
    }
}
