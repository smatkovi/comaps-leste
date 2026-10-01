import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// The content of the main window (MainWindow, a QMainWindow): the map and the pages on top of it.
// The window menu is the native one; it calls the functions at the bottom.
Item {
    id: window

    width: 420
    height: 820

    readonly property var currentPage: stack.currentItem
    readonly property var currentActions: currentPage && currentPage.pageActions ? currentPage.pageActions : []

    Binding {
        target: MapController
        property: "dark"
        value: Style.dark
    }

    Rectangle {
        anchors.fill: parent
        color: Metrics.background
    }

    MapView {
        id: map
        anchors.fill: parent
        primary: true
        visible: stack.depth <= 1 || stack.busy
        visibleArea: Qt.rect(0, content.y + chrome.obstructedTop, width,
                             Math.max(0, chrome.obstructedBottom - chrome.obstructedTop))
    }

    Item {
        id: frame
        anchors.fill: parent

        Item {
            id: content
            anchors.fill: parent

            StackView {
                id: stack
                anchors.fill: parent

                initialItem: MapChrome {
                    id: chrome
                }

                pushEnter: Transition {
                    NumberAnimation { property: "x"; from: stack.width * 0.25; to: 0; duration: 220; easing.type: Easing.OutCubic }
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 220 }
                }
                pushExit: Transition {
                    NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 180 }
                }
                popEnter: Transition {
                    NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 220 }
                }
                popExit: Transition {
                    NumberAnimation { property: "x"; from: 0; to: stack.width * 0.25; duration: 220; easing.type: Easing.OutCubic }
                    NumberAnimation { property: "opacity"; from: 1; to: 0; duration: 220 }
                }
            }

            Toast {
                id: toast
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: Metrics.spacingLarge
                width: Math.min(parent.width - 2 * Metrics.pageMargin, 480)
                z: 950
            }

            ActionSheet {
                id: actionSheet
                cancelLabel: qsTr("Cancel")
                onPicked: function (option) { AppShell.sheetPicked(option) }
            }

            MapLayersSheet {
                id: layersSheet
            }

            ConfirmSheet {
                id: confirmSheet
                cancelLabel: qsTr("Cancel")
                onConfirmed: AppShell.confirmAccepted()
            }

            ConfirmSheet {
                id: promptSheet

                property string placeholder: ""
                property alias text: promptField.text

                cancelLabel: qsTr("Cancel")
                confirmEnabled: promptField.text.trim().length > 0
                onShownChanged: if (shown) promptField.forceActiveFocus()
                onConfirmed: AppShell.promptAccepted(promptField.text)

                extraContent: [
                    TextField {
                        id: promptField
                        Layout.fillWidth: true
                        placeholderText: promptSheet.placeholder
                        onAccepted: if (promptSheet.confirmEnabled) {
                            promptSheet.close()
                            AppShell.promptAccepted(promptField.text)
                        }
                    }
                ]
            }
        }
    }

    Shortcut {
        sequences: ["Back", "Esc"]
        onActivated: AppShell.back()
    }

    Component.onCompleted: {
        AppShell.stack = stack
        AppShell.toastHandler = function (text, isError) { toast.show(text, isError) }
        AppShell.actionSheet = actionSheet
        AppShell.confirmSheet = confirmSheet
        AppShell.promptSheet = promptSheet
        AppShell.layersSheet = layersSheet
        AppShell.mapBackHandler = () => chrome.back()
    }

    // Called from the window menu.
    function showMapSearch() { AppShell.showMapSearch() }
}
