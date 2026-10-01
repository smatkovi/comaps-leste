import QtQuick
import QtQuick.Layouts

// Asks to confirm an action. extraContent goes between the message and the buttons.
SheetFrame {
    id: root

    property string title
    property string message
    property string confirmLabel: qsTr("OK")
    property string cancelLabel: qsTr("Cancel")
    property bool destructive: false
    property bool confirmEnabled: true
    property alias extraContent: extra.data

    signal confirmed()

    ColumnLayout {
        width: root.bodyWidth
        spacing: Metrics.spacing

        Text {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: root.title
            color: Style.text
            wrapMode: Text.Wrap
            font.pixelSize: Metrics.fontLarge
            font.weight: Font.DemiBold
        }

        Text {
            Layout.fillWidth: true
            visible: root.message !== ""
            horizontalAlignment: Text.AlignHCenter
            text: root.message
            color: Style.textMuted
            wrapMode: Text.Wrap
            font.pixelSize: Metrics.fontNormal
        }

        ColumnLayout {
            id: extra
            Layout.fillWidth: true
            visible: children.length > 0
            spacing: Metrics.spacingSmall
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Metrics.spacingSmall
            spacing: Metrics.spacing

            ActionButton {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                label: root.cancelLabel
                onClicked: root.close()
            }

            ActionButton {
                Layout.fillWidth: true
                Layout.preferredWidth: 1
                primary: true
                accentColor: root.destructive ? Style.danger : Style.accent
                enabled: root.confirmEnabled
                label: root.confirmLabel
                onClicked: {
                    root.close()
                    root.confirmed()
                }
            }
        }
    }
}
