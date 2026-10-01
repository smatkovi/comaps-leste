import QtQuick
import QtQuick.Layouts

CardPage {
    id: page

    property string categoryId: ""

    title: categoryId === "" ? qsTr("Bookmarks") : bookmarks.categoryName
    icon: categoryId === "" ? "bookmarks" : "folder_open"
    pageActions: categoryId === "" ? [
        { text: qsTr("New list…"), trigger: page.newList }
    ] : [
        { text: qsTr("Show all on map"), trigger: () => page.showOnMap(page.categoryId, "category") },
        { text: qsTr("Rename list…"), trigger: () => page.rename(page.categoryId, "category", bookmarks.categoryName) }
    ]

    BookmarkModel {
        id: bookmarks
        categoryId: page.categoryId
    }

    EmptyState {
        visible: bookmarks.count === 0
        glyph: page.categoryId === "" ? "bookmarks" : "bookmark_border"
        title: page.categoryId === "" ? qsTr("No bookmark lists") : qsTr("This list is empty")
        hint: qsTr("Save places from the place page to find them here.")
    }

    ActionButton {
        Layout.fillWidth: true
        visible: page.categoryId === ""
        glyph: "create_new_folder"
        label: qsTr("New list")
        onClicked: page.newList()
    }

    SettingsCard {
        Layout.fillWidth: true
        visible: bookmarks.count > 0

        Repeater {
            model: bookmarks

            delegate: SettingsNavRow {
                id: row

                required property int index
                required property string itemId
                required property string kind
                required property string name
                required property string detail
                required property bool shown

                glyph: kind === "category" ? "folder_open" : kind === "track" ? "route" : "bookmark"
                label: name
                value: detail
                chevron: kind === "category"
                divider: index < bookmarks.count - 1

                IconButton {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: row.kind === "category"
                    iconName: row.shown ? "visibility" : "visibility_off"
                    color: row.shown ? Style.accentOnSurface : Style.textDim
                    onClicked: bookmarks.setShown(row.itemId, !row.shown)
                }

                IconButton {
                    anchors.verticalCenter: parent.verticalCenter
                    iconName: "more_vert"
                    onClicked: page.showActions(row.itemId, row.kind, row.name)
                }

                onTapped: {
                    if (kind === "category")
                        AppShell.openBookmarkList(itemId)
                    else
                        page.showOnMap(itemId, kind)
                }
            }
        }
    }

    function showOnMap(id, kind) {
        bookmarks.show(id, kind)
        AppShell.popToMap()
    }

    function newList() {
        AppShell.prompt(qsTr("New list"), qsTr("List name"), "", qsTr("Create"), name => bookmarks.createList(name))
    }

    function rename(id, kind, name) {
        AppShell.prompt(qsTr("Rename"), qsTr("Name"), name, qsTr("Rename"), newName => bookmarks.rename(id, kind, newName))
    }

    function showActions(id, kind, name) {
        AppShell.sheet(name, "", [
            { label: qsTr("Show on map"), glyph: "map", action: () => page.showOnMap(id, kind) },
            { label: qsTr("Rename"), glyph: "edit", action: () => page.rename(id, kind, name) },
            { label: qsTr("Delete"), glyph: "delete", destructive: true,
              action: () => AppShell.confirm(qsTr("Delete %1?").arg(name),
                                             kind === "category" ? qsTr("All bookmarks and tracks in this list are deleted.") : "",
                                             qsTr("Delete"), true, () => bookmarks.remove(id, kind)) }
        ])
    }
}
