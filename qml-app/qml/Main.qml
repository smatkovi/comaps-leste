import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform as Platform

ApplicationWindow {
    id: window

    width: 420
    height: 820
    visible: true
    title: qsTr("Maps")
    color: Metrics.background

    readonly property var currentPage: stack.currentItem
    readonly property var currentActions: currentPage && currentPage.pageActions ? currentPage.pageActions : []

    Binding {
        target: MapController
        property: "dark"
        value: Style.dark
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

    Platform.MenuBar {
        // No title: the items sit at the top level of the menu.
        Platform.Menu {

            Platform.MenuItem {
                text: qsTr("Map")
                onTriggered: AppShell.popToMap()
            }

            Platform.MenuItem {
                text: qsTr("Search")
                onTriggered: AppShell.openSearch()
            }

            Platform.MenuSeparator {}

            Platform.MenuItem {
                text: qsTr("Bookmarks")
                onTriggered: AppShell.openBookmarks()
            }

            Platform.MenuItem {
                text: qsTr("Map downloads")
                onTriggered: AppShell.openDownloader("")
            }

            Platform.MenuSeparator {}

            Platform.MenuItem {
                text: qsTr("Settings")
                onTriggered: AppShell.openSettings()
            }

            Platform.MenuItem {
                text: qsTr("About")
                onTriggered: AppShell.openAbout()
            }
        }

        Platform.Menu {
            id: pageMenu
            title: window.currentPage && window.currentPage.title ? window.currentPage.title : qsTr("Page")
            visible: window.currentActions.length > 0

            Instantiator {
                model: window.currentActions
                onObjectAdded: function (index, object) { pageMenu.insertItem(index, object) }
                onObjectRemoved: function (index, object) { pageMenu.removeItem(object) }

                delegate: Platform.MenuItem {
                    required property var modelData
                    text: modelData.text
                    checkable: modelData.checkable === true
                    checked: modelData.checked === true
                    enabled: modelData.enabled !== false
                    onTriggered: if (modelData.trigger) modelData.trigger()
                }
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
}
