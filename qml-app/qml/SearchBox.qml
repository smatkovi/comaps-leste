import QtQuick

// A rounded search field. edited is emitted for every change of the text, accepted on Enter.
Rectangle {
    id: root

    property alias text: input.text
    property alias placeholderText: placeholder.text

    signal edited(string value)
    signal accepted()

    function focusInput() {
        input.forceActiveFocus()
    }

    implicitWidth: 240 * Metrics.scale
    implicitHeight: Metrics.buttonHeight
    radius: height / 2
    color: Style.surfaceRaised
    border.width: 1
    border.color: input.activeFocus ? Style.accent : Style.line

    AppIcon {
        id: searchIcon
        anchors.left: parent.left
        anchors.leftMargin: Metrics.spacing
        anchors.verticalCenter: parent.verticalCenter
        name: "search"
        size: Metrics.fontLarge
        color: Style.textDim
    }

    TextInput {
        id: input
        anchors.left: searchIcon.right
        anchors.right: clearButton.visible ? clearButton.left : parent.right
        anchors.leftMargin: Metrics.spacingSmall
        anchors.rightMargin: clearButton.visible ? 0 : Metrics.spacing
        anchors.verticalCenter: parent.verticalCenter
        enabled: root.enabled
        clip: true
        color: Style.text
        selectionColor: Style.accent
        selectedTextColor: Style.accentInk
        selectByMouse: true
        font.pixelSize: Metrics.fontNormal
        onTextChanged: root.edited(text)
        onAccepted: root.accepted()
    }

    Text {
        id: placeholder
        anchors.left: input.left
        anchors.right: input.right
        anchors.verticalCenter: parent.verticalCenter
        visible: input.text.length === 0
        color: Style.textDim
        elide: Text.ElideRight
        font.pixelSize: Metrics.fontNormal
    }

    IconButton {
        id: clearButton
        anchors.right: parent.right
        anchors.rightMargin: Metrics.spacingTight
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: root.height - 2 * Metrics.spacingTight
        visible: root.enabled && input.text.length > 0
        iconName: "close"
        iconSize: Metrics.fontLarge
        color: Style.textDim
        onClicked: {
            input.text = ""
            input.forceActiveFocus()
        }
    }
}
