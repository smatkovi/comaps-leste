import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: page

    property string title: qsTr("Search")
    readonly property var pageActions: results.count > 0 ? [
        { text: qsTr("Show results on map"), trigger: page.showAll }
    ] : []

    SearchModel {
        id: results
    }

    Rectangle {
        anchors.fill: parent
        color: Metrics.background
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: Metrics.pageMargin
        anchors.rightMargin: Metrics.pageMargin
        anchors.topMargin: Metrics.spacing
        spacing: Metrics.spacing

        RowLayout {
            Layout.fillWidth: true
            // A layout fills its height by default: without this the field is handed the space the
            // results leave and sits in the middle of it instead of at the top.
            Layout.fillHeight: false
            spacing: Metrics.spacingSmall

            IconButton {
                visible: Metrics.needsBackButton
                Layout.leftMargin: -Metrics.spacingSmall
                iconName: "arrow_back"
                onClicked: AppShell.back()
            }

            SearchBox {
                id: field
                Layout.fillWidth: true
                placeholderText: qsTr("Search Maps")
                onEdited: function (value) { results.query = value }
                onAccepted: Qt.inputMethod.hide()
            }
        }

        CategoryList {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: field.text.length === 0
            categories: [
                { title: qsTr("Nearby"), items: [
                    { key: "category_eat", icon: "restaurant", label: qsTr("Places to eat") },
                    { key: "category_food", icon: "local_grocery_store", label: qsTr("Groceries") },
                    { key: "category_fuel", icon: "local_gas_station", label: qsTr("Fuel") },
                    { key: "category_parking", icon: "local_parking", label: qsTr("Parking") },
                    { key: "category_transport", icon: "directions_transit", label: qsTr("Transport") },
                    { key: "category_hotel", icon: "hotel", label: qsTr("Hotels") }
                ] },
                { title: qsTr("Services"), items: [
                    { key: "category_atm", icon: "local_atm", label: qsTr("ATMs") },
                    { key: "category_pharmacy", icon: "local_pharmacy", label: qsTr("Pharmacies") },
                    { key: "category_hospital", icon: "local_hospital", label: qsTr("Hospitals") },
                    { key: "category_toilet", icon: "wc", label: qsTr("Toilets") },
                    { key: "category_wifi", icon: "wifi", label: qsTr("Wi-Fi") }
                ] }
            ]
            onPicked: function (key) {
                results.searchCategory(key)
                field.text = results.query
                Qt.inputMethod.hide()
            }
        }

        Item {
            id: resultsArea
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: field.text.length > 0

            EmptyState {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                visible: results.count === 0
                glyph: results.searching ? "travel_explore" : "search_off"
                title: results.searching ? qsTr("Searching…") : qsTr("No results")
                hint: results.searching ? "" : qsTr("Download the map of this area to search it offline.")
            }

            SettingsCard {
                id: groupsCard
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                visible: results.groups.length > 0

                Repeater {
                    model: results.groups

                    delegate: SettingsNavRow {
                        required property int index
                        required property var modelData

                        glyph: modelData.glyph
                        label: modelData.label
                        value: modelData.hint
                        divider: index < results.groups.length - 1
                        onTapped: page.showGroup(index)
                    }
                }
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: groupsCard.visible ? groupsCard.bottom : parent.top
                anchors.topMargin: groupsCard.visible ? Metrics.spacing : 0
                height: Math.min(list.contentHeight, parent.height - y - Metrics.spacing)
                visible: results.count > 0
                radius: Metrics.radius
                color: Style.surface
                clip: true

                ListView {
                    id: list
                    anchors.fill: parent
                    model: results
                    boundsBehavior: Flickable.StopAtBounds

                    delegate: SettingsNavRow {
                        id: row

                        required property int index
                        required property string name
                        required property string type
                        required property string address
                        required property string distance
                        required property bool suggest

                        width: ListView.view.width
                        height: rowHeight
                        glyph: suggest ? "search" : "place"
                        label: name
                        value: [type, address].filter(s => s !== "").join(" · ")
                        chevron: false
                        divider: index < results.count - 1

                        Text {
                            anchors.verticalCenter: parent.verticalCenter
                            visible: row.distance !== ""
                            text: row.distance
                            color: Style.textDim
                            font.pixelSize: Metrics.fontSmall
                        }

                        onTapped: page.activate(index)
                    }
                }
            }
        }
    }

    StackView.onActivated: field.focusInput()

    function activate(index) {
        const suggestion = results.activate(index)
        if (suggestion !== "") {
            field.text = suggestion
            return
        }
        Qt.inputMethod.hide()
        AppShell.popToMap()
    }

    function showAll() {
        results.showResultsOnMap()
        Qt.inputMethod.hide()
        AppShell.popToMap()
    }

    function showGroup(index) {
        results.showGroupOnMap(index)
        Qt.inputMethod.hide()
        AppShell.popToMap()
    }
}
