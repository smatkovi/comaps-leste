import QtQuick
import QtQuick.Layouts

Item {
    id: page

    default property alias content: column.data
    property alias trailing: trailingRow.data
    property alias footer: footerColumn.data
    property string title: ""
    property string icon: ""
    property int contentSpacing: Metrics.spacing
    property bool showBack: Metrics.needsBackButton
    property var pageActions: []
    readonly property Flickable flickable: flick

    signal backRequested()

    onBackRequested: AppShell.back()

    Rectangle {
        anchors.fill: parent
        color: Metrics.background
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: titleRow.implicitHeight + Metrics.spacing + Metrics.headerGap

            RowLayout {
                id: titleRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Metrics.headerGap
                spacing: Metrics.spacingSmall * 0.75

                IconButton {
                    visible: page.showBack
                    Layout.leftMargin: -Metrics.spacingSmall
                    iconName: "arrow_back"
                    onClicked: page.backRequested()
                }

                AppIcon {
                    visible: page.icon !== ""
                    name: page.icon
                    size: Metrics.fontLarge
                    color: Style.accent
                }

                Text {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    text: page.title
                    color: Style.text
                    font.pixelSize: Metrics.fontLarge
                    font.weight: Font.DemiBold
                }

                RowLayout {
                    id: trailingRow
                    spacing: Metrics.spacingSmall
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Style.line
        }

        Flickable {
            id: flick
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: column.implicitHeight + Metrics.spacingLarge
            topMargin: Metrics.spacingSmall
            boundsBehavior: Flickable.StopAtBounds
            clip: true
            interactive: contentHeight > height

            ColumnLayout {
                id: column
                width: flick.width
                spacing: page.contentSpacing
            }
        }

        ColumnLayout {
            id: footerColumn
            Layout.fillWidth: true
            Layout.topMargin: children.length > 0 ? Metrics.spacingSmall : 0
            Layout.bottomMargin: children.length > 0 ? Metrics.spacing : 0
            spacing: Metrics.spacingSmall
            visible: children.length > 0
        }
    }
}
