import QtQuick
import QtQuick.Layouts

// A list of options ({index, label, glyph, destructive, enabled, checked}) to pick one from.
SheetFrame {
    id: root

    property string title
    property string message
    property string cancelLabel: qsTr("Cancel")
    property var options: []

    signal picked(var option)

    ColumnLayout {
        width: root.bodyWidth
        spacing: Metrics.spacing

        Text {
            Layout.fillWidth: true
            visible: root.title !== ""
            horizontalAlignment: Text.AlignHCenter
            text: root.title
            color: Style.text
            elide: Text.ElideRight
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

        SettingsCard {
            Layout.fillWidth: true
            visible: root.options.length > 0

            Repeater {
                model: root.options

                delegate: Item {
                    id: option

                    required property var modelData
                    required property int index

                    readonly property color tone: modelData.destructive ? Style.danger : Style.text

                    Layout.fillWidth: true
                    implicitHeight: Metrics.rowHeight
                    enabled: modelData.enabled
                    opacity: enabled ? 1.0 : 0.45

                    Rectangle {
                        anchors.fill: parent
                        color: Style.press
                        opacity: tap.pressed ? 1 : 0
                        Behavior on opacity { NumberAnimation { duration: Metrics.pressDuration } }
                    }

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: Metrics.sideInset
                        anchors.rightMargin: Metrics.sideInset
                        spacing: Metrics.spacing

                        AppIcon {
                            Layout.preferredWidth: Metrics.iconSize
                            visible: option.modelData.glyph !== ""
                            name: option.modelData.glyph
                            color: option.modelData.destructive ? Style.danger : Style.accentOnSurface
                        }

                        Text {
                            Layout.fillWidth: true
                            text: option.modelData.label
                            color: option.tone
                            elide: Text.ElideRight
                            font.pixelSize: Metrics.fontNormal
                        }

                        AppIcon {
                            visible: option.modelData.checked
                            name: "check"
                            color: Style.accentOnSurface
                        }
                    }

                    Rectangle {
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.bottom: parent.bottom
                        anchors.leftMargin: Metrics.sideInset
                        height: 1
                        visible: option.index < root.options.length - 1
                        color: Style.line
                    }

                    TapHandler {
                        id: tap
                        gesturePolicy: TapHandler.ReleaseWithinBounds
                        onTapped: {
                            root.close()
                            root.picked(option.modelData)
                        }
                    }
                }
            }
        }

        ActionButton {
            Layout.fillWidth: true
            label: root.cancelLabel
            onClicked: root.close()
        }
    }
}
