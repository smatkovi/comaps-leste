pragma Singleton

import QtQuick
import QtQuick.Controls

QtObject {
    id: shell

    property StackView stack: null
    property var toastHandler: null
    property var actionSheet: null
    property var confirmSheet: null
    property var promptSheet: null
    property var layersSheet: null
    property var mapBackHandler: null

    readonly property bool onMap: stack === null || stack.depth <= 1
    // The search bar on the map is shown (search mode).
    property bool mapSearchShown: false

    // The search bar on the map should take the focus.
    signal mapSearchRequested()

    property var _sheetHandlers: []
    property var _confirmHandler: null
    property var _promptHandler: null

    function push(page, properties) {
        if (!stack)
            return
        stack.push(Qt.resolvedUrl(page), properties === undefined ? {} : properties)
    }

    function pop() {
        if (stack && stack.depth > 1)
            stack.pop()
    }

    function popToMap() {
        if (stack && stack.depth > 1)
            stack.pop(null)
    }

    function back() {
        for (const sheet of [actionSheet, confirmSheet, promptSheet, layersSheet]) {
            if (sheet && sheet.shown) {
                sheet.close()
                return
            }
        }
        if (stack && stack.depth > 1) {
            stack.pop()
            return
        }
        if (mapBackHandler)
            mapBackHandler()
    }

    function openSearch(query) { push("SearchPage.qml", query ? { initialQuery: query } : {}) }
    function showMapSearch() {
        popToMap()
        mapSearchShown = true
        mapSearchRequested()
    }
    function openDownloader(parentId) { push("DownloaderPage.qml", { parentId: parentId || "" }) }
    function openBookmarks() { push("BookmarksPage.qml") }
    function openBookmarkList(categoryId) { push("BookmarksPage.qml", { categoryId: categoryId }) }
    function openSettings() { push("SettingsPage.qml") }
    function openAbout() { push("AboutPage.qml") }

    function toast(text) {
        if (toastHandler)
            toastHandler(text, false)
    }

    function error(text) {
        if (text && toastHandler)
            toastHandler(text, true)
    }

    function sheet(title, message, options) {
        if (!actionSheet)
            return
        const visibleOptions = options.filter(option => option && option.visible !== false)
        shell._sheetHandlers = visibleOptions.map(option => option.action)
        actionSheet.title = title || ""
        actionSheet.message = message || ""
        actionSheet.options = visibleOptions.map((option, index) => ({
            index: index,
            label: option.label,
            glyph: option.glyph || "",
            destructive: option.destructive === true,
            enabled: option.enabled !== false,
            checked: option.checked === true
        }))
        actionSheet.open()
    }

    function sheetPicked(option) {
        const handler = shell._sheetHandlers[option.index]
        shell._sheetHandlers = []
        if (handler)
            handler()
    }

    function confirm(title, message, confirmLabel, destructive, onConfirm) {
        if (!confirmSheet)
            return
        shell._confirmHandler = onConfirm
        confirmSheet.title = title
        confirmSheet.message = message || ""
        confirmSheet.confirmLabel = confirmLabel || qsTr("OK")
        confirmSheet.destructive = destructive === true
        confirmSheet.open()
    }

    function confirmAccepted() {
        const handler = shell._confirmHandler
        shell._confirmHandler = null
        if (handler)
            handler()
    }

    function prompt(title, placeholder, initialText, confirmLabel, onAccept) {
        if (!promptSheet)
            return
        shell._promptHandler = onAccept
        promptSheet.title = title
        promptSheet.confirmLabel = confirmLabel || qsTr("OK")
        promptSheet.placeholder = placeholder || ""
        promptSheet.text = initialText || ""
        promptSheet.open()
    }

    function promptAccepted(text) {
        const handler = shell._promptHandler
        shell._promptHandler = null
        if (handler && text.trim().length > 0)
            handler(text.trim())
    }
}
