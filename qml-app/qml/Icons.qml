pragma Singleton

import QtQuick

// Icons are drawn with the Material Icons font (fonts-material-design-icons-iconfont), whose
// ligatures turn an icon's name into its glyph.
QtObject {
    readonly property string family: "Material Icons"

    // Names the font does not have, mapped to the closest one it does.
    readonly property var renames: ({})

    function glyph(name) {
        return renames[name] || name || ""
    }
}
