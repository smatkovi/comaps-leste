import QtQuick

// A surface filled with the accent colour, slightly graded from top to bottom.
Rectangle {
    id: root

    property color baseColor: Style.accent

    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.lighter(root.baseColor, 1.08) }
        GradientStop { position: 1.0; color: Qt.darker(root.baseColor, 1.12) }
    }
}
