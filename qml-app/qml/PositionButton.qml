import QtQuick

MapButton {
    readonly property int mode: MapController.myPositionMode

    lit: mode === MapController.Follow || mode === MapController.FollowAndRotate
    glyph: mode === MapController.FollowAndRotate ? "navigation"
         : mode === MapController.NoPosition ? "location_disabled"
         : mode === MapController.PendingPosition ? "location_searching"
         : "my_location"
    onTapped: MapController.nextMyPositionMode()

    SequentialAnimation on opacity {
        running: MapController.myPositionMode === MapController.PendingPosition
        loops: Animation.Infinite
        alwaysRunToEnd: true
        NumberAnimation { to: 0.45; duration: 600; easing.type: Easing.InOutQuad }
        NumberAnimation { to: 1; duration: 600; easing.type: Easing.InOutQuad }
    }
}
